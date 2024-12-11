/*
    После рендеринга глифа библиотекой FreeType структура FT_BitmapGlyph
    конвертируется в данный класс
*/

#pragma once

#include <ft2build.h>
#include FT_GLYPH_H

#include <dviglo/common/primitive_types.hpp>
#include <dviglo/res/image.hpp>

using namespace dviglo;


class GrayscaleImage
{
private:
    i32 width_;
    i32 height_;

    // Grayscale пиксели изображения, сконвертированные в диапазон 0.f - 1.f
    f32* pixels_;

public:
    // Конструктор копирует данные из FT_BitmapGlyph. FT_BitmapGlyph - это указатель
    GrayscaleImage(const FT_BitmapGlyph bitmap_glyph);

    // Копирование
    GrayscaleImage(const GrayscaleImage& other);
    GrayscaleImage& operator=(const GrayscaleImage& other);

    // Перемещение
    GrayscaleImage(GrayscaleImage&& other) noexcept;
    GrayscaleImage& operator=(GrayscaleImage&& other) noexcept;

    ~GrayscaleImage();

    f32 get_pixel(i32 x, i32 y) const;
    void set_pixel(i32 x, i32 y, f32 gray);

    // Некоторые эффекты требуют увеличения размера битмапа.
    // Битмап расширяется в каждую сторону на border_size, то есть ширина и высота увеличатся на border_size * 2.
    // Старое изображение располагается в центре нового
    void expand(i32 border_size);

    // Размазывает каждый пиксель на radius в каждую сторону (без учета исходного пикселя).
    // Размеры итогового изображения увеличатся на radius * 2 по вертикали и по горизонтали
    void blur(i32 radius);

    // Проверяет, что координаты не выходят за границы изображения
    bool is_inside(i32 x, i32 y) { return x >= 0 && x < width_ && y >= 0 && y < height_; }

    Image to_image() const;
};
