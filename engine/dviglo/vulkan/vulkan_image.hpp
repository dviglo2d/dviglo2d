// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "vulkan_utils.hpp"


namespace dviglo
{

// Меняет раскладку изображения
void transition(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout old_layout, vk::ImageLayout new_layout);

class VulkanImage
{
    VmaAllocatedImage   allocated_image_;
    vk::UniqueImageView view_;
    vk::Extent2D        extent_;
    vk::ImageLayout     layout_;

    VulkanImage() = default;

public:
    static std::optional<VulkanImage> create(const vma::Allocator& vma_allocator, glm::uvec2 size);

    vk::Image       image()  const noexcept { return allocated_image_.second.get(); }
    vk::ImageView   view()   const noexcept { return view_.get(); }
    vk::Extent2D    extent() const noexcept { return extent_; }
    vk::ImageLayout layout() const noexcept { return layout_; }

    // Меняет раскладку изображения
    void transition(vk::CommandBuffer cmd, vk::ImageLayout new_layout);

    // Перезаписывает текущий layout, а потом меняет раскладку изображения
    void transition(vk::CommandBuffer cmd, vk::ImageLayout old_layout, vk::ImageLayout new_layout);
};

// Нельзя копировать
static_assert(!std::is_copy_constructible_v<VulkanImage>);
static_assert(!std::is_copy_assignable_v<VulkanImage>);

// Можно перемещать
static_assert(std::is_move_constructible_v<VulkanImage>);
static_assert(std::is_move_assignable_v<VulkanImage>);

} // namespace dviglo
