// Copyright (c) the Dviglo project
// License: MIT

#include "texture_set.hpp"

#include "../main/graphics.hpp"

#include <dv_log.hpp>

using namespace glm;
using namespace std;


namespace dviglo
{

// TODO: queue_family_index не используется
TextureSet::TextureSet(vma::Allocator vma_allocator, vk::Queue queue, u32 queue_family_index)
    : vma_allocator_(vma_allocator)
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    vk::Device vk_device = vma_allocator.getAllocatorInfo().device;
    vk::Result vk_result;

    // Изображение - белый пиксель
    {
        optional<VulkanImage> optional_texture = VulkanImage::create(uvec2(1, 1), vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

        if (!optional_texture)
        {
            Log::writef_error("{} | !optional_texture", DV_FUNC_SIG);
            return;
        }

        vk::Image image = optional_texture.value().allocated_image.second.get();

        // Перевалочный буфер с белым пикселем
        VmaAllocatedBuffer staging_buffer;

        {
            // 0xAABBGGRR интрпретируется как 4 байта r g b a, так как little-endian
            const u32 white_pixel = 0xFFFFFFFF;

            vk::BufferCreateInfo buffer_create_info
            {
                .size = sizeof(white_pixel),
                .usage = vk::BufferUsageFlagBits::eTransferSrc,
                .sharingMode = vk::SharingMode::eExclusive,
            };

            vma::AllocationCreateInfo vma_allocation_create_info
            {
                .flags = vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
                .usage = vma::MemoryUsage::eAuto,
                .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            };

            tie(vk_result, staging_buffer) = vma_allocator.createBufferUnique(buffer_create_info, vma_allocation_create_info).asTuple();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | vma_allocator.createBufferUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                return;
            }

            void* mapped_data = vma_allocator.getAllocationInfo(staging_buffer.first.get()).pMappedData;
            memcpy(mapped_data, &white_pixel, sizeof(white_pixel));
        }

        // Выделяем командный буфер из пула
        vk::UniqueCommandBuffer cmd;

        {
            vk::CommandBufferAllocateInfo command_buffer_allocate_info
            {
                .commandPool = DV_GRAPHICS->transient_command_pool(),
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1,
            };

            vector<vk::UniqueCommandBuffer> cmds;
            tie(vk_result, cmds) = vk_device.allocateCommandBuffersUnique(command_buffer_allocate_info).asTuple();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | vk_device.allocateCommandBuffers(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                return;
            }

            cmd = std::move(cmds.front());
        }

        // Заполняем командный буфер командами
        {
            vk::CommandBufferBeginInfo command_buffer_begin_info
            {
                .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
            };

            vk_result = cmd->begin(command_buffer_begin_info);

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | cmd->begin(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                return;
            }

            // Меняем раскладку изображения
            transition(*cmd, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

            // Копируем данные из перевалочного буфера в видеопамять
            vk::BufferImageCopy buffer_image_copy
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
                .imageExtent = { 1, 1, 1 }
            };

            cmd->copyBufferToImage(staging_buffer.second.get(), image, vk::ImageLayout::eTransferDstOptimal, 1, &buffer_image_copy);

            // Меняем раскладку изображения
            transition(*cmd, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

            vk_result = cmd->end();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | cmd->end() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                return;
            }
        }

        // Выполняем команды
        {
            vk::SubmitInfo submit_info
            {
                .commandBufferCount = 1,
                .pCommandBuffers = &cmd.get(),
            };

            vk_result = queue.submit(submit_info, nullptr);

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | queue.submit(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                return;
            }

            // CPU ждёт GPU
            vk_result = queue.waitIdle();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | queue.waitIdle(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                return;
            }
        }

        white_pixel_ = std::move(optional_texture.value());
    }

    is_valid_ = true;
}

TextureSet::~TextureSet()
{
    instance_ = nullptr;
}

const VulkanImage& TextureSet::get(const fs::path& path, vk::Format format)
{
    fs::path normal_path = path.lexically_normal();

    // Возвращаем из кэша, если уже загружено
    {
        auto it = umap_storage_.find(normal_path);
        if (it != umap_storage_.end())
            return it->second;
    }

    // Загружаем
    {
        std::optional<VulkanImage> texture = VulkanImage::load(path, format);

        //if (!texture)
        // TODO

        auto [it, inserted] = umap_storage_.try_emplace(normal_path, std::move(texture.value()));

        return it->second;
    }
}

} // namespace dviglo
