// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "vulkan_utils.hpp"


namespace dviglo
{

void transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout);

struct VulkanImage
{
    VmaAllocatedImage   allocated_image;
    vk::UniqueImageView view;
    vk::Extent2D        extent;
    vk::ImageLayout     layout = vk::ImageLayout::eUndefined;

    static std::optional<VulkanImage> create(const vma::Allocator& vma_allocator, glm::uvec2 size);

    vk::Image image() const noexcept { return allocated_image.second.get(); }

    // Меняет раскладку изображения
    void transition(vk::CommandBuffer cmd, vk::ImageLayout new_layout);

    // layout = vk::ImageLayout::eUndefined, а потом меняет раскладку изображения
    void transition_from_undefined(vk::CommandBuffer cmd, vk::ImageLayout new_layout);
};

// Нельзя копировать
static_assert(!std::is_copy_constructible_v<VulkanImage>);
static_assert(!std::is_copy_assignable_v<VulkanImage>);

// Можно перемещать
static_assert(std::is_move_constructible_v<VulkanImage>);
static_assert(std::is_move_assignable_v<VulkanImage>);

} // namespace dviglo
