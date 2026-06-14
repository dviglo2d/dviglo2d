// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "vulkan_image.hpp"

#include <dv_primitive_types.hpp>


namespace dviglo
{

// Рендерим сначала в offscreen (закадровое, внеэкранное) изображение, а потом переносим
// в swapchain изображение с коррекцией гаммы.
// TODO: описать падение с rivatuner
class Swapchain
{
private:
    // Поля расположены в порядке инициализации

    vk::PresentModeKHR present_mode_ = vk::PresentModeKHR::eImmediate;
    vk::Extent2D swapchain_image_extent_;
    vk::SurfaceFormatKHR surface_format_;

    vk::UniqueSwapchainKHR value_;

    // Изображения уничтожать не надо, будут уничтожены при уничтожении value_
    std::vector<vk::Image> swapchain_images_;

    std::vector<vk::UniqueImageView> swapchain_image_views_;

public:
    VulkanImage offscreen_image_;

private:

    // То что создавалось каждый кадр
    vk::UniqueSampler sampler_;

    vk::UniqueDescriptorSetLayout dsl_;

    vk::UniqueDescriptorPool pool_;
    vk::UniqueDescriptorSet unique_desc_set_;

    vk::UniquePipelineLayout pipeline_layout;
    vk::UniquePipeline pipeline;

    // Отдельный статический командный буфер для каждого изображения.
    // Будут уничтожены при уничтожении Graphics::persistent_command_pool_
    std::vector<vk::CommandBuffer> command_buffers_;

    vk::UniqueFence fence;

    Swapchain() = default;

public:
    static std::optional<Swapchain> construct(vk::PhysicalDevice physical_device, vk::Device device,
                                              u32 graphics_queue_family_index, u32 present_queue_family_index,
                                              vk::SurfaceKHR surface,
                                              glm::uvec2 window_size,
                                              glm::uvec2 offscreen_size,
                                              vma::Allocator& vma_allocator,
                                              vk::PresentModeKHR present_mode);

    // Запрещаем копировать объект
    Swapchain(const Swapchain&)             = delete;
    Swapchain& operator =(const Swapchain&) = delete;

    // Но разрешаем перемещать
    Swapchain(Swapchain&& other)             noexcept = default;
    Swapchain& operator =(Swapchain&& other) noexcept = default;

    vk::PresentModeKHR present_mode() const { return present_mode_; }
    vk::Extent2D swapchain_image_extent() const { return swapchain_image_extent_; }
    vk::Extent2D offscreen_image_extent() const { return offscreen_image_.extent; }
    vk::SurfaceFormatKHR surface_format() const { return surface_format_; }
    vk::SwapchainKHR get() const { return *value_; }
    const std::vector<vk::UniqueImageView>& swapchain_image_views() const { return swapchain_image_views_; }

    // Запрашивает у Vulkan следующее изображение.
    // fence - забор, который Vulkan откроет, когда изображение станет доступно.
    // Возвращает код результата и индекс картинки (если успешно)
    std::tuple<vk::Result, u32> acquire_next_image(vk::Device device, vk::Fence fence);

    // Отправляет отрендеренное изображение на экран.
    // wait_semaphore - семафор (обычно render_finished), который нужно дождаться перед показом
    vk::Result present(vk::Queue graphics_queue, vk::Queue present_queue, u32 image_index, vk::Device device, vk::CommandPool command_pool, vk::Semaphore wait_semaphore = nullptr);
};

}  // namespace dviglo
