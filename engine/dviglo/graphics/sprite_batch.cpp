// Copyright (c) the Dviglo project
// License: MIT

#include "sprite_batch.hpp"


namespace dviglo
{

SpriteBatch::SpriteBatch(OffscreenImage&& offscreen_image)
    : offscreen_image_(std::move(offscreen_image))
{
}

std::optional<SpriteBatch> SpriteBatch::create(const vma::Allocator& vma_allocator, glm::uvec2 size)
{
    std::optional<OffscreenImage> optional_offscreen_image = OffscreenImage::create(vma_allocator, size);

    if (!optional_offscreen_image)
        return std::nullopt; // Сообщение об ошибке уже выведено

    SpriteBatch ret(std::move(*optional_offscreen_image));

    return ret;
}

} // namespace dviglo
