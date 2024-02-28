// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../common/primitive_types.hpp"

#include <glm/glm_wrapped.hpp>


namespace dviglo
{

struct Rect
{
    glm::vec2 pos;
    glm::vec2 size;

    Rect() = default;

    Rect(glm::vec2 pos, glm::vec2 size)
        : pos(pos)
        , size(size)
    {
    }

    Rect(f32 x, f32 y, f32 width, f32 height)
        : pos(x, y)
        , size(width, height)
    {
    }
};

struct IntRect
{
    glm::ivec2 pos;
    glm::ivec2 size;

    IntRect() = default;

    IntRect(glm::ivec2 pos, glm::ivec2 size)
        : pos(pos)
        , size(size)
    {
    }

    IntRect(i32 x, i32 y, i32 width, i32 height)
        : pos(x, y)
        , size(width, height)
    {
    }
};

} // namespace dviglo
