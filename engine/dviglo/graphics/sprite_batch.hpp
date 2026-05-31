// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../vulkan/offscreen_image.hpp"


namespace dviglo
{

class SpriteBatch
{
private:
    OffscreenImage offscreen_image_;

    SpriteBatch(OffscreenImage&& offscreen_image);

public:
    static std::optional<SpriteBatch> create(const vma::Allocator& vma_allocator, glm::uvec2 size);
};

// Нельзя копировать
static_assert(!std::is_copy_constructible_v<SpriteBatch>);
static_assert(!std::is_copy_assignable_v<SpriteBatch>);

// Можно перемещать
static_assert(std::is_move_constructible_v<SpriteBatch>);
static_assert(std::is_move_assignable_v<SpriteBatch>);

} // namespace dviglo
