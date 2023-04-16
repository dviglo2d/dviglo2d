// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../gl_utils/texture.hpp"
#include "../std_utils/string.hpp"

#include <limits>
#include <unordered_map>


namespace dviglo
{

struct Glyph
{
    i16 x = 0; ///< Горизонтальная позиция в текстуре
    i16 y = 0; ///< Вертикальаня позиция в текстуре
    i16 width = 0; ///< Ширина
    i16 height = 0; ///< Высота
    i16 offset_x = 0; ///< Смещение от origin (левый верхний угол) по горизонтали при рендеринге
    i16 offset_y = 0; ///< Смещение от origin (левый верхний угол) по вертикали при рендеринге
    i16 advance_x = 0; ///< Расстояние между origin (левый верхний угол) текущего глифа и origin следующего глифа при рендеринге
    i16 page = std::numeric_limits<i16>::max(); /// Номер текстуры
};

class SpriteFont
{
private:
    StrUtf8 face_; // Название исходного шрифта (из которого был сгенерирован растровый шрифт)
    i32 size_ = 0; // Размер исходного шрифта
    i32 line_height_ = 0; // Высота растрового шрифта
    std::vector<Texture*> textures_; // Текстурные атласы с символами
    std::unordered_map<c32, Glyph> glyphs_; // кодовая позиция -> изображение

public:
    SpriteFont(const SpriteFont&) = delete;
    SpriteFont& operator=(const SpriteFont&) = delete;

    Texture* texture(size_t index)
    {
        return textures_[index];
    }

    const Glyph& glyph(u32 code_point)
    {
        return glyphs_[code_point];
    }

    i32 line_height() const { return line_height_; }

    SpriteFont(const StrUtf8& file_path);
};

} // namespace dviglo
