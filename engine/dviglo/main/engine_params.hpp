// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/string.hpp"

#include <glm/glm.hpp>


namespace dviglo
{

enum class WindowMode : u32
{
    windowed = 0,
    resizable,
    fullscreen_window,
    exclusive_fullscreen
};


// Эти параметры используются при инициализации движка
namespace engine_params
{
    extern StrUtf8 log_path;

    inline StrUtf8 window_title{"Игра"};
    inline glm::ivec2 window_size{800, 600};
    inline WindowMode window_mode = WindowMode::windowed;

    // 0 - выключено, 1 - включено,
    // -1 - адаптивная вертикальная синхронизация (если не поддерживается, то включается обычная).
    // Значения 2, -2, 3, -3 и т.д. делят частоту кадров
    inline i32 vsync = 0;

    // 0 или 1 - MSAA выключено,
    // другое значение - число сэмплов (рекомендуется 4 или 8).
    // Подробнее: https://habr.com/ru/articles/351706/
    inline i32 msaa_samples = 0;
}

} // namespace dviglo
