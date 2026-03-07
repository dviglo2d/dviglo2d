// Copyright (c) the Dviglo project
// License: MIT

#include "swapchain.hpp"

#include <dv_log.hpp>

using namespace glm;
using namespace std;


namespace dviglo
{

std::optional<Swapchain> Swapchain::construct(vk::PhysicalDevice physical_device, vk::Device device,
                                              u32 graphics_queue_family_index, u32 present_queue_family_index,
                                              vk::SurfaceKHR surface, glm::ivec2 window_size,
                                              vk::PresentModeKHR present_mode)
{
    Swapchain ret;

    assert(window_size.x > 0 && window_size.y > 0);

    vk::Result vk_result;

    // ============================= Режим показа =============================
    {
        vector<vk::PresentModeKHR> present_modes;
        tie(vk_result, present_modes) = physical_device.getSurfacePresentModesKHR(surface);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | physical_device.getSurfacePresentModesKHR(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }

        ret.present_mode_ = present_mode;

        // Если запрошенный режим не поддерживается, то пробуем похожий.
        // Спецификация гарантирует поддержку только vk::PresentModeKHR::eFifo
        if (!ranges::contains(present_modes, ret.present_mode_))
        {
            // VK_PRESENT_MODE_IMMEDIATE_KHR - нет VSync, артефакты, минимальный инпут лаг
            if (ret.present_mode_ == vk::PresentModeKHR::eImmediate)
            {
                if (ranges::contains(present_modes, vk::PresentModeKHR::eMailbox))
                {
                    // VK_PRESENT_MODE_MAILBOX_KHR - новые кадры рендерятся постоянно и CPU/GPU загружены на 100%.
                    // При синхроимпульсе отображается самый последний отрендеренный кадр.
                    // Тиринга нет, инпут лаг чуть больше, чем при VK_PRESENT_MODE_IMMEDIATE_KHR
                    ret.present_mode_ = vk::PresentModeKHR::eMailbox;
                }
                else if (ranges::contains(present_modes, vk::PresentModeKHR::eFifoRelaxed))
                {
                    // VK_PRESENT_MODE_FIFO_RELAXED_KHR - есть VSync, однако, если кадр рендерился слишком долго и синхроимпульс был пропущен,
                    // то отрендеренный кадр будет выведен сразу, не дожидаясь следующего синхроимпульса.
                    // Это предотвращает микрофризы как в VK_PRESENT_MODE_FIFO_KHR, но может привести к тирингу.
                    // Инпут лаг высокий, как при VK_PRESENT_MODE_FIFO_KHR
                    ret.present_mode_ = vk::PresentModeKHR::eFifoRelaxed;
                }
                else
                {
                    // VK_PRESENT_MODE_FIFO_KHR - есть VSync, нет тиринга, высокий инпут лаг.
                    // Если кадр рендерится долго и синхроимпульс был пропущен, то показ откладывается до следующего синхроимпульса
                    // (например, FPS падает с 60 до 30)
                    ret.present_mode_ = vk::PresentModeKHR::eFifo;
                }
            }
            else if (ret.present_mode_ == vk::PresentModeKHR::eMailbox)
            {
                if (ranges::contains(present_modes, vk::PresentModeKHR::eImmediate))
                    ret.present_mode_ = vk::PresentModeKHR::eImmediate;
                else if (ranges::contains(present_modes, vk::PresentModeKHR::eFifoRelaxed))
                    ret.present_mode_ = vk::PresentModeKHR::eFifoRelaxed;
                else
                    ret.present_mode_ = vk::PresentModeKHR::eFifo;
            }
            else // present_mode_ == vk::PresentModeKHR::eFifoRelaxed
            {
                ret.present_mode_ = vk::PresentModeKHR::eFifo;
            }
        }
    }

    vk::SurfaceCapabilitiesKHR surface_capabilities;
    tie(vk_result, surface_capabilities) = physical_device.getSurfaceCapabilitiesKHR(surface);

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | physical_device.getSurfaceCapabilitiesKHR(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return std::nullopt;
    }

    // ============================= Размер изображений =============================
    {
        // Если Vulkan требует определённый размер
        if (surface_capabilities.currentExtent.width != numeric_limits<u32>::max())
        {
            ret.image_extent_ = surface_capabilities.currentExtent;
        }
        else
        {
            ret.image_extent_.width = glm::clamp(static_cast<u32>(window_size.x),
                                                 surface_capabilities.minImageExtent.width,
                                                 surface_capabilities.maxImageExtent.width);

            ret.image_extent_.height = glm::clamp(static_cast<u32>(window_size.y),
                                                  surface_capabilities.minImageExtent.height,
                                                  surface_capabilities.maxImageExtent.height);
        }

    }

    // ============================= Число изображений =============================

    u32 image_count = 0;

    {
        if (ret.present_mode_ == vk::PresentModeKHR::eFifo || ret.present_mode_ == vk::PresentModeKHR::eFifoRelaxed)
        {
            // Избегаем очереди кадров, так как это повышает инпут лаг
            image_count = 2;
        }
        else if (ret.present_mode_ == vk::PresentModeKHR::eMailbox)
        {
            // Для тройной буферизации всегда должен быть 1 свободный буфер.
            // На Android minImageCount может быть 3, на ПК всегда 2
            image_count = glm::max<u32>(3, surface_capabilities.minImageCount + 1);
        }
        else // vk::PresentModeKHR::eImmediate
        {
            image_count = surface_capabilities.minImageCount + 1;
        }

        if (image_count < surface_capabilities.minImageCount)
            image_count = surface_capabilities.minImageCount;

        // maxImageCount может быть 0, если лимита нет
        if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount)
            image_count = surface_capabilities.maxImageCount;
    }
            
    // ============================= Формат изображений =============================
    {
        vector<vk::SurfaceFormatKHR> surface_formats;
        tie(vk_result, surface_formats) = physical_device.getSurfaceFormatsKHR(surface);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | physical_device.getSurfaceFormatsKHR(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }

        // TODO: eB8G8R8A8Srgb крэшится при запущенном RivaTuner
        //ret.surface_format_ = { vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear };
        ret.surface_format_ = { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };

        if (!ranges::contains(surface_formats, ret.surface_format_))
        {
            Log::writef_error("{} | !ranges::contains(surface_formats, surface_format_)", DV_FUNC_SIG);
            return std::nullopt;
        }
    }

    // ============================= Свопчейн =============================
    {
        vk::SwapchainCreateInfoKHR swapchain_info
        {
            .surface = surface,
            .minImageCount = image_count,
            .imageFormat = ret.surface_format_.format,
            .imageColorSpace = ret.surface_format_.colorSpace,
            .imageExtent = ret.image_extent_,
            .imageArrayLayers = 1, // Всегда 1, если это не стерео/VR
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment, // Мы будем рисовать в это изображение
            .preTransform = surface_capabilities.currentTransform,
            .presentMode = ret.present_mode_,
            .clipped = vk::True,
        };

        u32 queues[] = { graphics_queue_family_index, present_queue_family_index };

        // Если эти два семейства очередей - одно семейство
        if (graphics_queue_family_index == present_queue_family_index)
        {
            swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
        }
        else
        {
            swapchain_info.imageSharingMode = vk::SharingMode::eConcurrent;
            swapchain_info.setQueueFamilyIndices(queues);
        }

        tie(vk_result, ret.value_) = device.createSwapchainKHRUnique(swapchain_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | device.createSwapchainKHRUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    // ============================= Изображения =============================
    {
        tie(vk_result, ret.images_) = device.getSwapchainImagesKHR(*ret.value_);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | device.getSwapchainImagesKHR(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    // ============================= Вьюхи =============================
    {
        ret.image_views_.reserve(ret.images_.size());

        for (const vk::Image image : ret.images_)
        {
            vk::ImageViewCreateInfo image_view_info
            {
                .image = image,
                .viewType = vk::ImageViewType::e2D,
                .format = ret.surface_format_.format, // TODO: Тут менять формат на sRGB

                .subresourceRange =
                {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };

            vk::UniqueImageView image_view;
            tie(vk_result, image_view) = device.createImageViewUnique(image_view_info).asTuple();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | device.createImageViewUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                return std::nullopt;
            }

            ret.image_views_.push_back(std::move(image_view));
        }
    }

    return ret;
}

}  // namespace dviglo
