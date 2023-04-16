// Copyright (c) the Dviglo project
// License: MIT

#include "vulkan_image.hpp"

#include "../main/graphics.hpp"
#include "../res/image.hpp"

#include <dv_log.hpp>


namespace dviglo
{

void transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout)
{
    if (old_layout == new_layout)
    {
        Log::writef_warning("{} | old_layout == new_layout | {}", DV_FUNC_SIG, vk::to_string(old_layout));
        return;
    }

    vk::ImageMemoryBarrier2 barrier
    {
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .image = image,

        .subresourceRange = vk::ImageSubresourceRange
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    if (old_layout == vk::ImageLayout::eUndefined)
    {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eNone;
        barrier.srcAccessMask = vk::AccessFlagBits2::eNone;
    }
    else if (old_layout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    }
    else if (old_layout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
        barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
    }
    else if (old_layout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
        barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
    }
    else
    {
        Log::writef_error("{} | Unexpected old_layout | {}", DV_FUNC_SIG, vk::to_string(old_layout));
        return;
    }

    if (new_layout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
        barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    }
    else if (new_layout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
        barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
    }
    else if (new_layout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
        barrier.dstAccessMask = vk::AccessFlagBits2::eTransferRead;
    }
    else if (new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
        barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
    }
    else if (new_layout == vk::ImageLayout::ePresentSrcKHR)
    {
        barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
    }
    else
    {
        Log::writef_error("{} | Unexpected new_layout | {}", DV_FUNC_SIG, vk::to_string(new_layout));
        return;
    }

    vk::DependencyInfo dependency_info
    {
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    cmd.pipelineBarrier2(dependency_info);
}

std::optional<VulkanImage> VulkanImage::create(glm::uvec2 size, vk::ImageUsageFlags usage, vk::Format format)
{
    VulkanImage ret;
    vk::Result vk_result;

    {
        ret.extent = { size.x, size.y };
        // ret.layout уже проинициализирован
        ret.format = format;
    }

    // ============================= VmaAllocatedImage allocated_image =============================
    {
        vk::ImageCreateInfo image_create_info
        {
            .imageType = vk::ImageType::e2D,
            .format = ret.format,
            .extent = { size.x, size.y, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined,
        };

        vma::AllocationCreateInfo vma_allocation_create_info
        {
            .usage = vma::MemoryUsage::eAuto,
        };

        std::tie(vk_result, ret.allocated_image) = DV_GRAPHICS->vma_allocator().createImageUnique(image_create_info, vma_allocation_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vma_allocator.createImageUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    // ============================= vk::UniqueImageView view =============================
    {
        vk::ImageViewCreateInfo image_view_create_info
        {
            .image = ret.allocated_image.second.get(),
            .viewType = vk::ImageViewType::e2D,
            .format = ret.format,

            .subresourceRange = vk::ImageSubresourceRange
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };

        std::tie(vk_result, ret.view) = DV_GRAPHICS->device().createImageViewUnique(image_view_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | device.createImageViewUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    return ret;
}

std::optional<VulkanImage> VulkanImage::load(const fs::path& path, vk::Format format)
{
    VulkanImage ret;
    vk::Result vk_result;

    Image image(path, true);

    // TODO: Пустое изображение надо обрабатывать иначе
    if (image.num_components() < 1 || image.num_components() > 4)
    {
        Log::writef_error("{} | image->num_components() == {}", DV_FUNC_SIG, image.num_components());
        return std::nullopt;
    }

    // Создаем изображение с флагами TransferDst и Sampled
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;

    auto vulkan_image_opt = create(glm::uvec2(image.width(), image.height()), usage, format);
    if (!vulkan_image_opt)
    {
        Log::writef_error("{} | Failed to create VulkanImage", DV_FUNC_SIG);
        return std::nullopt;
    }

    ret = std::move(*vulkan_image_opt);

    // Вычисляем размер данных в байтах
    size_t data_size = image.width() * image.height() * image.num_components();

    // Создаем staging buffer
    vk::BufferCreateInfo buffer_create_info
    {
        .size = data_size,
        .usage = vk::BufferUsageFlagBits::eTransferSrc,
    };

    vma::AllocationCreateInfo vma_alloc_create_info
    {
        .usage = vma::MemoryUsage::eCpuToGpu,
    };

    auto [buf_result, staging_buffer] = DV_GRAPHICS->vma_allocator().createBufferUnique(
        buffer_create_info, vma_alloc_create_info);

    if (buf_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | Failed to create staging buffer | {}", DV_FUNC_SIG, vk::to_string(buf_result));
        return std::nullopt;
    }

    // Копируем данные пикселей в staging buffer
    void* mapped_data = nullptr;
    vk_result = DV_GRAPHICS->vma_allocator().mapMemory(staging_buffer.first.get(), &mapped_data);
    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | Failed to map memory | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return std::nullopt;
    }

    memcpy(mapped_data, image.data(), data_size);
    DV_GRAPHICS->vma_allocator().unmapMemory(staging_buffer.first.get());

    // Создаем одноразовый command buffer для копирования
    vk::CommandBufferAllocateInfo cmd_alloc_info
    {
        .commandPool = DV_GRAPHICS->transient_command_pool(),
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };

    auto [cmd_result, cmd_buffers] = DV_GRAPHICS->device().allocateCommandBuffersUnique(cmd_alloc_info);
    if (cmd_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | Failed to allocate command buffer | {}", DV_FUNC_SIG, vk::to_string(cmd_result));
        return std::nullopt;
    }

    vk::UniqueCommandBuffer cmd = std::move(cmd_buffers[0]);

    // Начинаем запись команд
    vk::CommandBufferBeginInfo begin_info
    {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    vk_result = cmd->begin(begin_info);
    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | Failed to begin command buffer | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return std::nullopt;
    }

    // Переводим изображение в layout для получения данных
    ret.transition_from_undefined(*cmd, vk::ImageLayout::eTransferDstOptimal);

    // Копируем данные из staging buffer в изображение
    vk::BufferImageCopy copy_region
    {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = vk::ImageSubresourceLayers
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = { 0, 0, 0 },
        .imageExtent = { static_cast<uint32_t>(image.width()), static_cast<uint32_t>(image.height()), 1 },
    };

    cmd->copyBufferToImage(
        staging_buffer.second.get(),
        ret.vk_image(),
        vk::ImageLayout::eTransferDstOptimal,
        1,
        &copy_region
    );

    // Переводим изображение в layout для чтения в шейдере
    ret.transition(*cmd, vk::ImageLayout::eShaderReadOnlyOptimal);

    cmd->end();

    // Отправляем команды на выполнение
    vk::SubmitInfo submit_info
    {
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd.get(),
    };

    vk_result = DV_GRAPHICS->graphics_queue().submit(1, &submit_info, nullptr);
    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | Failed to submit commands | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return std::nullopt;
    }

    // Ждем завершения копирования
    DV_GRAPHICS->graphics_queue().waitIdle();

    return ret;
}

void VulkanImage::transition(vk::CommandBuffer cmd, vk::ImageLayout new_layout)
{
    ::dviglo::transition(cmd, vk_image(), layout, new_layout);
    layout = new_layout;
}

void VulkanImage::transition_from_undefined(vk::CommandBuffer cmd, vk::ImageLayout new_layout)
{
    layout = vk::ImageLayout::eUndefined;
    ::dviglo::transition(cmd, vk_image(), layout, new_layout);
    layout = new_layout;
}

} // namespace dviglo
