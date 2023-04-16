// Copyright (c) the Dviglo project
// License: MIT

#include "offscreen_image.hpp"

#include <dv_log.hpp>


namespace dviglo
{

std::optional<OffscreenImage> OffscreenImage::create(const vma::Allocator& vma_allocator, glm::uvec2 size)
{
    OffscreenImage ret;
    vk::Result vk_result;
    vk::Device device = vma_allocator.getAllocatorInfo().device;

    // ============================= VmaAllocatedImage allocated_image =============================
    {
        vk::ImageCreateInfo image_create_info
        {
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eB8G8R8A8Unorm,
            .extent = { size.x, size.y, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined,
        };

        vma::AllocationCreateInfo vma_allocation_create_info
        {
            .usage = vma::MemoryUsage::eAuto,
        };

        std::tie(vk_result, ret.allocated_image) = vma_allocator.createImageUnique(image_create_info, vma_allocation_create_info).asTuple();

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
            .format = vk::Format::eB8G8R8A8Unorm,

            .subresourceRange = vk::ImageSubresourceRange
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };

        std::tie(vk_result, ret.view) = device.createImageViewUnique(image_view_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | device.createImageViewUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    // ============================= vk::UniqueRenderPass render_pass =============================
    {
        // render_pass_create_info.pAttachments[0]
        vk::AttachmentDescription color_attachment_desc
        {
            .format = vk::Format::eB8G8R8A8Unorm,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,

            // Раскладка прикрепления до render pass не имеет значения
            .initialLayout = vk::ImageLayout::eUndefined,

            // Раскладка прикрепления после render pass
            .finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        };

        // subpass_desc.pColorAttachments[0]
        // Соответствует layout(location = 0) out vec4 out_color; во фрагментном шейдере
        vk::AttachmentReference color_attachment_ref
        {
            // render_pass_create_info.pAttachments[0]
            .attachment = 0,

            // Раскладка прикрепления в процессе подпрохода
            .layout = vk::ImageLayout::eColorAttachmentOptimal,
        };

        vk::SubpassDescription subpass_desc
        {
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref,
        };

        vk::RenderPassCreateInfo render_pass_create_info
        {
            .attachmentCount = 1,
            .pAttachments = &color_attachment_desc,
            .subpassCount = 1,
            .pSubpasses = &subpass_desc,
        };

        std::tie(vk_result, ret.render_pass) = device.createRenderPassUnique(render_pass_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | device.createRenderPassUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    // ============================= vk::Framebuffer framebuffer =============================
    {
        vk::FramebufferCreateInfo framebuffer_create_info
        {
            .renderPass = ret.render_pass.get(),
            .attachmentCount = 1,
            .pAttachments = &ret.view.get(),
            .width = size.x,
            .height = size.y,
            .layers = 1,
        };

        std::tie(vk_result, ret.framebuffer) = device.createFramebufferUnique(framebuffer_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | device.createFramebufferUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return std::nullopt;
        }
    }

    // ============================= Остальные поля =============================
    {
        ret.extent = { size.x, size.y };
    }

    return ret;
}

} // namespace dviglo
