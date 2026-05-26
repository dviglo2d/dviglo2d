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

    // Проход рендера с предварительной очисткой. В конце переводит раскладку изображения
    // в vk::ImageLayout::eShaderReadOnlyOptimal
    vk::UniqueRenderPass render_pass;

    vk::UniqueFramebuffer framebuffer;
    vk::Extent2D          extent;

    static std::optional<OffscreenImage> create(const vma::Allocator& vma_allocator, glm::uvec2 size);

    vk::Image image() const noexcept { return allocated_image.second.get(); }
};

// Нет конструктора копирования
static_assert(!std::is_copy_constructible_v<OffscreenImage>);

// Нет оператора копирования
static_assert(!std::is_copy_assignable_v<OffscreenImage>);

// Есть конструктор перемещения
static_assert(std::is_move_constructible_v<OffscreenImage>);

// Есть оператор перемещения
static_assert(std::is_move_assignable_v<OffscreenImage>);

} // namespace dviglo
