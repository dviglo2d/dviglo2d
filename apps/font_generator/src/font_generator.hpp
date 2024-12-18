#pragma once

#include "glyph_renderer.hpp"


struct GeneratedFont
{
    vector<Image> pages;

    GeneratedFont() = default;

    // Запрещаем копирование
    GeneratedFont(const RenderedGlyph&) = delete;
    GeneratedFont& operator=(const RenderedGlyph&) = delete;

    // Разрешаем перемещение
    GeneratedFont(GeneratedFont&&) = default;
    GeneratedFont& operator=(GeneratedFont&&) = default;
};

GeneratedFont generate_font_simple(const FreeTypeFace& face, const FontSettings& font_settings);
