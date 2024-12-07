#pragma once

#include "grayscale_image.hpp"

#include <dviglo/std_utils/string.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include <memory>

using namespace std;


struct RenderedGlyph
{
    unique_ptr<GrayscaleImage> grayscale_image;

    // Запрещаем копирование
    RenderedGlyph(const RenderedGlyph&) = delete;
    RenderedGlyph& operator=(const RenderedGlyph&) = delete;

    // Разрешаем перемещение
    RenderedGlyph(RenderedGlyph&& other) noexcept = default;
    RenderedGlyph& operator=(RenderedGlyph&& other) noexcept = default;

    // Символ в кодировке UTF-32
    u32 code_point;

    // Смещение при выводе на экран
    i32 x_offset;
    i32 y_offset;

    // Расстояние между origin текущего глифа и origin следующего глифа
    i32 x_advance;

    // Высота строки (одинакова для всех глифов)
    i32 line_height;
};




class GlyphRenderer
{
private:


    // Текущий шрифт
    FT_Face face_;

public:
    GlyphRenderer(const StrUtf8& font_path, i32 font_height);
};
