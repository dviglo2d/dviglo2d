// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "vulkan_utils.hpp"


namespace dviglo
{

// Закадровое (внеэкранное) изображение
struct OffscreenImage
{
    VmaAllocatedImage   allocated_image;
    vk::UniqueImageView view;
    vk::Extent2D        extent;

    static std::optional<OffscreenImage> create(const vma::Allocator& vma_allocator, glm::uvec2 size);

    vk::Image image() const noexcept { return allocated_image.second.get(); }
};

// Нельзя копировать
static_assert(!std::is_copy_constructible_v<OffscreenImage>);
static_assert(!std::is_copy_assignable_v<OffscreenImage>);

// Можно перемещать
static_assert(std::is_move_constructible_v<OffscreenImage>);
static_assert(std::is_move_assignable_v<OffscreenImage>);

} // namespace dviglo
