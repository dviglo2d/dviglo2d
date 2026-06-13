// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "vulkan_utils.hpp"


namespace dviglo
{

void transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout);

// Перевалочный буфер хранит изображение в ОЗУ, из него данные будут копироваться в видеопамять.
// Нельзя уничтожать буфер, пока все данные не будут скопированы
struct StagingBuffer
{
    vk::Buffer      buffer;
    vma::Allocation allocation;
};

// Структура помогает отслеживать раскладку изображения
// TODO: Переименовать в Texture
struct VulkanImage
{
    VmaAllocatedImage   allocated_image;
    vk::UniqueImageView view;
    vk::Extent2D        extent;
    vk::ImageLayout     layout = vk::ImageLayout::eUndefined;
    vk::Format          format = vk::Format::eR8G8B8A8Unorm;

    static std::optional<VulkanImage> create(glm::uvec2 size, vk::ImageUsageFlags usage);

    vk::Image vk_image() const noexcept { return allocated_image.second.get(); }

    // Меняет раскладку изображения
    void transition(vk::CommandBuffer cmd, vk::ImageLayout new_layout);

    // layout = vk::ImageLayout::eUndefined, а потом меняет раскладку изображения
    void transition_from_undefined(vk::CommandBuffer cmd, vk::ImageLayout new_layout);

    //void upload_sync(const vma::Allocator& vma_allocator, vk::CommandBuffer cmd, vk::Queue queue, const void* pixels, size_t size_bytes);
};

// Нельзя копировать
static_assert(!std::is_copy_constructible_v<VulkanImage>);
static_assert(!std::is_copy_assignable_v<VulkanImage>);

// Можно перемещать
static_assert(std::is_move_constructible_v<VulkanImage>);
static_assert(std::is_move_assignable_v<VulkanImage>);

} // namespace dviglo
