// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <glm/glm_wrapped.hpp>


namespace dviglo
{

struct Rect
{
    glm::vec2 min;
    glm::vec2 max;

    // Clang версии < 16 не поддерживает P0960R3. Позже оба конструктора можно будет удалить
    // https://en.cppreference.com/w/cpp/compiler_support/20

    Rect() = default;

    Rect(glm::vec2 min, glm::vec2 max)
        : min(min)
        , max(max)
    {
    }

    float get_width() const { return max.x - min.x; }
    float get_height() const { return max.y - min.y; }
};

struct IntRect
{
    glm::ivec2 min;
    glm::ivec2 max;

    // Clang версии < 16 не поддерживает P0960R3. Позже оба конструктора можно будет удалить
    // https://en.cppreference.com/w/cpp/compiler_support/20

    IntRect() = default;

    IntRect(glm::ivec2 min, glm::ivec2 max)
        : min(min)
        , max(max)
    {
    }

    i32 get_width() const { return max.x - min.x; }
    i32 get_height() const { return max.y - min.y; }
};

} // namespace dviglo
