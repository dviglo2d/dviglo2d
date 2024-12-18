#pragma once

#include <dviglo/fs/fs_base.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

using namespace dviglo;
using namespace glm;


enum class FontStyle : u32
{
    simple = 0,
    outlined,         // С обводкой
    contour,          // Только контур
    last = contour    // Для определения числа стилей
};


struct FontSettings
{
    StrUtf8 src_path = get_base_path() + "font_generator_data/fonts/ubuntu/Ubuntu-R.ttf";
    i32 height = 60; // В пикселях
    FontStyle font_style = FontStyle::simple;
    ImVec4 main_color{1.f, 1.f, 1.f, 1.f};   // Основной цвет
    ImVec4 second_color{0.f, 0.f, 0.f, 1.f}; // Дополнительный цвет
    i32 thickness   = 10; // Толщина контура или обводки
    i32 blur_radius = 8;  // Радиус размытия
    ivec2 texture_size{1024, 1024};
};
