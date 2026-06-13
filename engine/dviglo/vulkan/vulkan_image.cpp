// Copyright (c) the Dviglo project
// License: MIT

#include "vulkan_image.hpp"

#include "../main/graphics.hpp"

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

std::optional<VulkanImage> VulkanImage::create(glm::uvec2 size, vk::ImageUsageFlags usage)
{
    VulkanImage ret;
    vk::Result vk_result;

    {
        ret.extent = { size.x, size.y };
        // ret.layout и ret.format уже проинициализированы
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
