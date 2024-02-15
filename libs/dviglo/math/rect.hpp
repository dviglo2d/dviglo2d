// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <glm/glm_wrapped.hpp>


namespace dviglo
{

struct Rect
{
    glm::vec2 pos;
    glm::vec2 size;

    // Clang версии < 16 не поддерживает P0960R3. Позже оба конструктора можно будет удалить
    // https://en.cppreference.com/w/cpp/compiler_support/20

    Rect() = default;

    Rect(glm::vec2 pos, glm::vec2 size)
        : pos(pos)
        , size(size)
    {
    }
};

struct IntRect
{
    glm::ivec2 pos;
    glm::ivec2 size;

    // Clang версии < 16 не поддерживает P0960R3. Позже оба конструктора можно будет удалить
    // https://en.cppreference.com/w/cpp/compiler_support/20

    IntRect() = default;

    IntRect(glm::ivec2 pos, glm::ivec2 size)
        : pos(pos)
        , size(size)
    {
    }
};

} // namespace dviglo
