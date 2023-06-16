// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <glm/glm.hpp>


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
};

} // namespace dviglo
