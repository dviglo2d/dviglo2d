// Copyright (c) the Dviglo project
// License: MIT

#include "swapchain.hpp"

#include "../main/graphics.hpp"

#include <dv_log.hpp>

using namespace glm;
using namespace std;


namespace dviglo
{

std::optional<Swapchain> Swapchain::construct(vk::PhysicalDevice physical_device, vk::Device device,
                                              u32 graphics_queue_family_index, u32 present_queue_family_index,
                                              vk::SurfaceKHR surface,
                                              uvec2 window_size,
                                              uvec2 offscreen_size,
                                              vma::Allocator& vma_allocator,
                                              vk::PresentModeKHR present_mode)
{
    Swapchain ret;

    vk::Result vk_result;

    // ============================= vk::PresentModeKHR present_mode_ =============================
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

    // ============================= vk::Extent2D swapchain_image_extent_ =============================
    {
        // Если Vulkan требует определённый размер
        if (surface_capabilities.currentExtent.width != numeric_limits<u32>::max())
        {
            ret.swapchain_image_extent_ = surface_capabilities.currentExtent;
        }
        else
        {
            ret.swapchain_image_extent_.width = glm::clamp(static_cast<u32>(window_size.x),
                                                           surface_capabilities.minImageExtent.width,
                                                           surface_capabilities.maxImageExtent.width);

            ret.swapchain_image_extent_.height = glm::clamp(static_cast<u32>(window_size.y),
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
            
    // ============================= vk::SurfaceFormatKHR surface_format_ =============================
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

        // TODO: сперва проверить RGBA формат, а потом BGRA
    }

    // ============================= Свопчейн =============================
    {
        vk::SwapchainCreateInfoKHR swapchain_info
        {
            .surface = surface,
            .minImageCount = image_count,
            .imageFormat = ret.surface_format_.format,
            .imageColorSpace = ret.surface_format_.colorSpace,
            .imageExtent = ret.swapchain_image_extent_,
            .imageArrayLayers = 1, // Всегда 1, если это не стерео/VR
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment // Мы будем рисовать в это изображение
               | vk::ImageUsageFlagBits::eTransferDst, // Убрать это для теста
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

    // ============================= Изображения свопчейна =============================
    {
        tie(vk_result, ret.swapchain_images_) = device.getSwapchainImagesKHR(*ret.value_);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | device.getSwapchainImagesKHR(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    // ============================= Вьюхи изображений свопчейна =============================
    {
        ret.swapchain_image_views_.reserve(image_count);

        for (const vk::Image image : ret.swapchain_images_)
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

            ret.swapchain_image_views_.push_back(std::move(image_view));
        }
    }

    {
        std::optional<VulkanImage> opt = VulkanImage::create(offscreen_size, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment);

        if (!opt)
            return std::nullopt; // Сообщение в лог уже выведено

        ret.offscreen_image_ = std::move(opt.value());
    }

    // То что создавалоьс каждый кадр
    {

        // Сэмплер
        {
            vk::SamplerCreateInfo sampler_info{
                .magFilter = vk::Filter::eLinear,
                .minFilter = vk::Filter::eLinear,
                .mipmapMode = vk::SamplerMipmapMode::eLinear,
                .addressModeU = vk::SamplerAddressMode::eClampToEdge,
                .addressModeV = vk::SamplerAddressMode::eClampToEdge,
                .addressModeW = vk::SamplerAddressMode::eClampToEdge,
            };
            std::tie(vk_result, ret.sampler_) = device.createSamplerUnique(sampler_info).asTuple();
            assert(vk_result == vk::Result::eSuccess);
        }

        // Descriptor Set Layout
        {
            vk::DescriptorSetLayoutBinding binding{
                .binding = 0,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = 1,
                .stageFlags = vk::ShaderStageFlagBits::eFragment,
            };
            vk::DescriptorSetLayoutCreateInfo dsl_info{
                .bindingCount = 1,
                .pBindings = &binding,
            };
            std::tie(vk_result, ret.dsl_) = device.createDescriptorSetLayoutUnique(dsl_info).asTuple();
            assert(vk_result == vk::Result::eSuccess);
        }

        // Descriptor Pool

        {
            vk::DescriptorPoolSize pool_size{ vk::DescriptorType::eCombinedImageSampler, 1 };
            vk::DescriptorPoolCreateInfo pool_info{
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                .maxSets = 1,
                .poolSizeCount = 1,
                .pPoolSizes = &pool_size,
            };
            std::tie(vk_result, ret.pool_) = device.createDescriptorPoolUnique(pool_info).asTuple();
            assert(vk_result == vk::Result::eSuccess);
        }

        // Descriptor Set

        {
            vk::DescriptorSetAllocateInfo alloc_info{
                .descriptorPool = *ret.pool_,
                .descriptorSetCount = 1,
                .pSetLayouts = &*ret.dsl_,
            };
            std::vector<vk::UniqueDescriptorSet> sets;
            std::tie(vk_result, sets) = device.allocateDescriptorSetsUnique(alloc_info).asTuple();
            assert(vk_result == vk::Result::eSuccess);
            ret.unique_desc_set_ = std::move(sets[0]);
        }


        // Обновляем дескриптор
        {
            vk::DescriptorImageInfo image_info{
                .sampler = *ret.sampler_,
                .imageView = ret.offscreen_image_.view.get(),
                .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            };
            vk::WriteDescriptorSet write{
                .dstSet = ret.unique_desc_set_.get(),
                .dstBinding = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &image_info,
            };
            device.updateDescriptorSets(1, &write, 0, nullptr);
        }

        // Pipeline Layout
        {
            vk::PipelineLayoutCreateInfo pl_info{
                .setLayoutCount = 1,
                .pSetLayouts = &*ret.dsl_,
            };
            std::tie(vk_result, ret.pipeline_layout) = device.createPipelineLayoutUnique(pl_info).asTuple();
            assert(vk_result == vk::Result::eSuccess);
        }

        // Graphics Pipeline

        {
            fs::path base_path = get_base_path();
            vk::UniqueShaderModule vert_shader_module = load_shader(device, base_path / "engine_data/shaders/fullscreen_triangle.vert.spv");

            if (!vert_shader_module)
            {
                Log::writef_error("{} | !vert_shader_module", DV_FUNC_SIG); // TODO: Иссправить
                return std::nullopt;
            }

            vk::UniqueShaderModule frag_shader_module = load_shader(device, base_path / "engine_data/shaders/to_srgb.frag.spv");

            if (!frag_shader_module)
            {
                Log::writef_error("{} | !frag_shader_module", DV_FUNC_SIG); // TODO: Иссправить
                return std::nullopt;
            }

            vk::PipelineShaderStageCreateInfo stages[2] = {
                vk::PipelineShaderStageCreateInfo{
                    .stage = vk::ShaderStageFlagBits::eVertex,
                    .module = *vert_shader_module,
                    .pName = "main",
                },
                vk::PipelineShaderStageCreateInfo{
                    .stage = vk::ShaderStageFlagBits::eFragment,
                    .module = *frag_shader_module,
                    .pName = "main",
                }
            };

            vk::PipelineVertexInputStateCreateInfo vertex_input{};
            vk::PipelineInputAssemblyStateCreateInfo input_assembly{
                .topology = vk::PrimitiveTopology::eTriangleList,
            };

            float scale = std::min(static_cast<float>(ret.swapchain_image_extent_.width) / ret.offscreen_image_.extent.width,
                static_cast<float>(ret.swapchain_image_extent_.height) / ret.offscreen_image_.extent.height);
            float targetW = static_cast<float>(ret.offscreen_image_.extent.width) * scale;
            float targetH = static_cast<float>(ret.offscreen_image_.extent.height) * scale;
            float offsetX = (static_cast<float>(ret.swapchain_image_extent_.width) - targetW) / 2.0f;
            float offsetY = (static_cast<float>(ret.swapchain_image_extent_.height) - targetH) / 2.0f;

            vk::Viewport viewport{
            .x = offsetX,
            .y = offsetY,
            //.width = static_cast<float>(swapchain_image_extent_.width),
            //.height = static_cast<float>(swapchain_image_extent_.height),
            .width = targetW,
            .height = targetH,


                .minDepth = 0.0f,
            .maxDepth = 1.0f
            };
            //vk::Rect2D scissor{ .offset = {0,0}, .extent = swapchain_image_extent_ };
            vk::Rect2D scissor{};
            scissor.offset = { (int32_t)offsetX, (int32_t)offsetY };
            scissor.extent = { (uint32_t)targetW, (uint32_t)targetH };


            vk::PipelineViewportStateCreateInfo viewport_state{
                    .viewportCount = 1,
                    .pViewports = &viewport,
                    .scissorCount = 1,
                    .pScissors = &scissor,
            };
            vk::PipelineRasterizationStateCreateInfo rasterization{
                .polygonMode = vk::PolygonMode::eFill,
                .cullMode = vk::CullModeFlagBits::eNone,
                .frontFace = vk::FrontFace::eCounterClockwise,
                .lineWidth = 1.0f,
            };
            vk::PipelineMultisampleStateCreateInfo multisample{
                .rasterizationSamples = vk::SampleCountFlagBits::e1,
            };
            vk::PipelineColorBlendAttachmentState blend_attachment{};
            blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            vk::PipelineColorBlendStateCreateInfo color_blend{
                .attachmentCount = 1,
                .pAttachments = &blend_attachment,
            };

            vk::Format color_format = ret.surface_format_.format;
            vk::PipelineRenderingCreateInfo pipeline_rendering_info{
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &color_format,
            };


            vk::GraphicsPipelineCreateInfo pipeline_info{
                .pNext = &pipeline_rendering_info,
                .stageCount = 2,
                .pStages = stages,
                .pVertexInputState = &vertex_input,
                .pInputAssemblyState = &input_assembly,
                .pViewportState = &viewport_state,
                .pRasterizationState = &rasterization,
                .pMultisampleState = &multisample,
                .pColorBlendState = &color_blend,
                .layout = *ret.pipeline_layout,
            };

            std::tie(vk_result, ret.pipeline) = device.createGraphicsPipelineUnique(nullptr, pipeline_info).asTuple();
            assert(vk_result == vk::Result::eSuccess);
        }

        {
            vk::CommandBufferAllocateInfo cb_alloc_info{
                .commandPool = DV_GRAPHICS->persistent_command_pool(),
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = image_count,
            };
            std::tie(vk_result, ret.command_buffers_) = device.allocateCommandBuffers(cb_alloc_info).asTuple();
            assert(vk_result == vk::Result::eSuccess); // LOG


            // Сразу заполняем все
            for (size_t image_index = 0; image_index < image_count; ++image_index)
            {
                vk::CommandBuffer cmd = ret.command_buffers_[image_index];
                vk::Image swapchain_image = ret.swapchain_images_[image_index];

                vk::CommandBufferBeginInfo command_buffer_begin_info;
                vk_result = cmd.begin(command_buffer_begin_info);
                if (vk_result != vk::Result::eSuccess)
                {
                    Log::writef_error("{} | cmd.begin(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                    return std::nullopt;
                }

                transition(cmd, swapchain_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

                vk::RenderingAttachmentInfo color_attachment
                {
                    .imageView = ret.swapchain_image_views_[image_index].get(),
                    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = { vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} } }
                };

                vk::RenderingInfo rendering_info
                {
                    .renderArea = { {0, 0}, ret.swapchain_image_extent_ },
                    .layerCount = 1,
                    .colorAttachmentCount = 1,
                    .pColorAttachments = &color_attachment
                };

                cmd.beginRendering(rendering_info);
                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *ret.pipeline);
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *ret.pipeline_layout, 0, *ret.unique_desc_set_, nullptr);
                cmd.draw(3, 1, 0, 0);
                cmd.endRendering();

                transition(cmd, swapchain_image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);

                vk_result = cmd.end();
                if (vk_result != vk::Result::eSuccess)
                {
                    Log::writef_error("{} | cmd.end() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                    return std::nullopt;
                }
            }
        }

        {
            std::tie(vk_result, ret.fence) = device.createFenceUnique({}).asTuple();
            assert(vk_result == vk::Result::eSuccess);
        }


    }

    return ret;
}

// Получение изображения
std::tuple<vk::Result, u32> Swapchain::acquire_next_image(vk::Device device, vk::Fence fence)
{
    vk::Result result;
    u32 image_index = 0;

    tie(result, image_index) = device.acquireNextImageKHR(*value_, UINT64_MAX, nullptr, fence).asTuple();

    // Логируем ошибку, если это не штатная ситуация. 
    // eErrorOutOfDateKHR и eSuboptimalKHR — это нормальное явление при изменении размера окна,
    // их мы логировать как ошибку не будем, так как внешний код должен пересоздать Swapchain
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR && result != vk::Result::eErrorOutOfDateKHR)
        Log::writef_error("{} | device.acquireNextImageKHR(...) | {}", DV_FUNC_SIG, vk::to_string(result));

    return { result, image_index };
}

// Показ изображения
vk::Result Swapchain::present(vk::Queue graphics_queue, vk::Queue present_queue, u32 image_index, vk::Device device, vk::CommandPool command_pool, vk::Semaphore wait_semaphore)
{
    // Копируем из фреймбуфера в изображение свопчейна

    vk::Image swapchain_image = swapchain_images_[image_index];


    


    vk::Result vk_result;



 // --- Вывод offscreen-текстуры с преобразованием через to_srgb.frag -------------------------------

    vk::CommandBuffer cmd = command_buffers_[image_index];

// Сабмит (как в #if 1)
/*vk::SubmitInfo submit_info{
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
};
device.resetFences(1, &fence.get());

graphics_queue.submit(1, &submit_info, *fence);
device.waitForFences(1, &fence.get(), vk::True, UINT64_MAX); // TODO: А что если убрать*/


// Ожидаем завершения рендера сцены перед наложением гаммы
    vk::SemaphoreSubmitInfo wait_info{
        .semaphore = wait_semaphore,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput
    };

    vk::CommandBufferSubmitInfo cmd_info{ .commandBuffer = cmd };

    vk::SubmitInfo2 submit_info{
        .waitSemaphoreInfoCount = wait_semaphore ? 1u : 0u,
        .pWaitSemaphoreInfos = wait_semaphore ? &wait_info : nullptr,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_info,
        .signalSemaphoreInfoCount = 0,
        .pSignalSemaphoreInfos = nullptr
    };

    // Отправляем буфер и просим дернуть забор по завершению
    device.resetFences(1, &fence.get());
    graphics_queue.submit2(1, &submit_info, fence.get());

    // ЖЁСТКАЯ СИНХРОНИЗАЦИЯ : Ждём видеокарту на процессоре(уменьшает инпут - лаг, устраняя очередь кадров)
        vk::Result wait_res = device.waitForFences(1, &fence.get(), vk::True, UINT64_MAX);
    if (wait_res != vk::Result::eSuccess)
        Log::writef_error("{} | device.waitForFences(...) | {}", DV_FUNC_SIG, vk::to_string(wait_res));


////// ---------- Конец вывода offscreen текстуры


    // Выводим на экран

    vk::PresentInfoKHR present_info
    {
        //.waitSemaphoreCount = wait_semaphore ? 1u : 0u,
        //.pWaitSemaphores = wait_semaphore ? &wait_semaphore : nullptr,
        .swapchainCount = 1,
        .pSwapchains = &value_.get(),
        .pImageIndices = &image_index
    };

    vk::Result result = present_queue.presentKHR(present_info);

    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR && result != vk::Result::eErrorOutOfDateKHR)
    {
        Log::writef_error("{} | present_queue.presentKHR(...) | {}", DV_FUNC_SIG, vk::to_string(result));
    }

    return result;
}

}  // namespace dviglo
