// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"

#include <glm/glm_wrapped.hpp>


namespace dviglo
{

/// Эти параметры используются при инициализации движка
namespace engine_params
{
    inline StrUtf8 window_title{"Игра"};
    inline glm::ivec2 window_size{800, 600};

    /// 0 - выключено, 1 - включено,
    /// -1 - адаптивная вертикальная синхронизация (если не поддерживается, то включается обычная).
    /// Значения 2, -2, 3, -3 и т.д. делят частоту кадров
    inline i32 vsync = 0;
}

} // namespace dviglo
