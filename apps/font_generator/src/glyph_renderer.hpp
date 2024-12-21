#pragma once

#include "freetype_face.hpp"
#include "grayscale_image.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include <memory>

using namespace std;


struct RenderedGlyph
{
    unique_ptr<GrayscaleImage> grayscale_image;

    RenderedGlyph() = default;

    // Запрещаем копирование
    RenderedGlyph(const RenderedGlyph&) = delete;
    RenderedGlyph& operator=(const RenderedGlyph&) = delete;

    // Разрешаем перемещение
    RenderedGlyph(RenderedGlyph&&) = default;
    RenderedGlyph& operator=(RenderedGlyph&&) = default;

    // Символ в кодировке UTF-32
    u32 code_point = 0;

    // Смещение при выводе на экран
    i32 x_offset = 0;
    i32 y_offset = 0;

    // Расстояние между origin текущего глифа и origin следующего глифа
    i32 x_advance = 0;
};

RenderedGlyph render_glyph_simpe(FT_Face face, const FontSettings& font_settings);
