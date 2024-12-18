#include "grayscale_image.hpp"

#include <utility>

using namespace glm;


GrayscaleImage::GrayscaleImage(const FT_BitmapGlyph bitmap_glyph)
{
    width_ = bitmap_glyph->bitmap.width;
    height_ = bitmap_glyph->bitmap.rows;
    pixels_ = new f32[width_ * height_];

    // Если изображение монохромное
    if (bitmap_glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
    {
        // В монохромном изображении один пиксель занимает 1 бит (не байт).
        // pitch - это число байт, занимаемых одной линией изображения
        for (i32 y = 0; y < height_; ++y)
        {
            u8* src = bitmap_glyph->bitmap.buffer + bitmap_glyph->bitmap.pitch * y;
            f32* row_dest = pixels_ + y * width_;

            for (i32 x = 0; x < width_; ++x)
            {
                // 1) В одном байте умещается 8 пикселей. x >> 3 эквивалентно делению
                //    на 8 (0b1000 превращается в 0b1). Так мы получаем байт, в котором находится пиксель
                // 2) x & 7 - это остаток от деления на 8. Допустим x = 12 = 0b1100
                //    0b1100 & 0b0111 = 0b100 = 4. Так мы получаем номер бита внутри байта.
                // 3) 0x80 == 0b10000000. Единица внутри этого числа сдвигается на номер бита
                //    и побитовой операцией определяется значение бита
                row_dest[x] = (src[x >> 3] & (0x80 >> (x & 7))) ? 1.0f : 0.0f;
            }
        }
    }
    else
    {
        // В grayscale изображении каждый пиксель занимает один байт,
        // а значит pitch эквивалентен width.
        for (i32 i = 0; i < width_ * height_; ++i)
            pixels_[i] = (f32)bitmap_glyph->bitmap.buffer[i] / 255.0f;
    }
}

GrayscaleImage::GrayscaleImage(const GrayscaleImage& other)
{
    width_ = other.width_;
    height_ = other.height_;
    pixels_ = new f32[width_ * height_];
    memcpy(pixels_, other.pixels_, width_ * height_ * sizeof(f32));
}

GrayscaleImage& GrayscaleImage::operator=(const GrayscaleImage& other)
{
    if (this != &other)
    {
        delete[] pixels_;

        width_ = other.width_;
        height_ = other.height_;
        pixels_ = new f32[width_ * height_];
        memcpy(pixels_, other.pixels_, width_ * height_ * sizeof(f32));
    }

    return *this;
}

GrayscaleImage::GrayscaleImage(GrayscaleImage&& other) noexcept
    : width_(std::exchange(other.width_, 0))
    , height_(std::exchange(other.height_, 0))
    , pixels_(std::exchange(other.pixels_, nullptr))
{
}

GrayscaleImage& GrayscaleImage::operator=(GrayscaleImage&& other) noexcept
{
    if (this != &other)
    {
        width_= std::exchange(other.width_, 0);
        height_ = std::exchange(other.height_, 0);
        pixels_ = std::exchange(other.pixels_, nullptr);
    }

    return *this;
}

GrayscaleImage::~GrayscaleImage()
{
    delete[] pixels_;
}

f32 GrayscaleImage::get_pixel(i32 x, i32 y) const
{
    return pixels_[y * width_ + x];
}

void GrayscaleImage::set_pixel(i32 x, i32 y, f32 gray)
{
    pixels_[y * width_ + x] = gray;
}

void GrayscaleImage::expand(i32 border_size)
{
    border_size = abs(border_size);
    i32 new_width = width_ + border_size * 2;
    i32 new_height = height_ + border_size * 2;
    f32* new_pixels = new f32[new_width * new_height];
    memset(new_pixels, 0, new_width * new_height * sizeof(f32));

    for (i32 x = 0; x < width_; ++x)
    {
        i32 new_x = x + border_size;

        for (i32 y = 0; y < height_; ++y)
        {
            i32 new_y = y + border_size;
            new_pixels[new_y * new_width + new_x] = pixels_[y * width_ + x];
        }
    }

    delete[] pixels_;
    pixels_ = new_pixels;
    width_ = new_width;
    height_ = new_height;
}

// Упрощает обращение к пикселям в любом буфере, который по размеру
// равен width_ * height_ (то есть такого же размера, что и pixels_)
#define PIXEL(buffer, x, y) buffer[width_ * (y) + (x)]

// Пробовал все три алгоритма с http://www.gamedev.ru/code/articles/blur .
// Обычное усреднение (первый метод) выглядит уродливо, изображение как будто двоится.
// Гауссово размытие (третий метод) выглядит красиво, но интенсивность пикселей очень
// быстро и нелинейно затухает с расстоянием. Бо'льшая часть радиуса размытия почти
// полностью прозрачна и не видна. Приходится нереально накручивать радиус размытия,
// чтобы хоть немного увеличить видимую площадь эффекта. А это бьет по производительности.
// Поэтому используется треугольный закон распределения (второй метод)
void GrayscaleImage::blur(i32 radius)
{
    if (radius == 0)
        return;

    radius = abs(radius);
    expand(radius);
    f32* tmp_buffer = new f32[width_ * height_];

    // Пиксель на каждом проходе размывается в две стороны, и получается отрезок
    // длиной radius + 1 + radius.
    // У крайних пикселей вес самый маленький и равен 1.
    // У центрального пикселя вес самый большой и равен radius + 1.
    // Вес соседних пикселей отличается на 1.
    // Сумму весов всех пикселей в этом отрезке можно определить сразу
    i32 total_weight = (radius + 1) * (radius + 1);

    // Размываем по вертикали и сохраняем результат в tmp_buffer
    for (i32 x = 0; x < width_; ++x)
    {
        for (i32 y = 0; y < height_; ++y)
        {
            // Сразу записываем вклад центрального пикселя.
            // Его вес равен radius + 1
            f32 value = get_pixel(x, y) * (radius + 1);
            i32 dist = 1;

            while (dist <= radius)
            {
                i32 weight = 1 + radius - dist;

                // Пиксель вне изображения черный, ноль можно не плюсовать.
                // Так что тут все корректно
                if (is_inside(x, y + dist))
                    value += get_pixel(x, y + dist) * weight;

                if (is_inside(x, y - dist))
                    value += get_pixel(x, y - dist) * weight;

                dist++;
            }

            // Сумму нужно поделить на общий вес, иначе изменится яркость изображения
            PIXEL(tmp_buffer, x, y) = value / total_weight;
        }
    }

    // Размываем по горизонтали и сохраняем результат назад в структуру
    for (i32 x = 0; x < width_; ++x)
    {
        for (i32 y = 0; y < height_; ++y)
        {
            f32 value = PIXEL(tmp_buffer, x, y) * (radius + 1);
            i32 dist = 1;

            while (dist <= radius)
            {
                i32 weight = 1 + radius - dist;

                if (is_inside(x + dist, y))
                    value += PIXEL(tmp_buffer, x + dist, y) * weight;

                if (is_inside(x - dist, y))
                    value += PIXEL(tmp_buffer, x - dist, y) * weight;

                dist++;
            }

            set_pixel(x, y, value / total_weight);
        }
    }

    delete[] tmp_buffer;
}

Image GrayscaleImage::to_image() const
{
    Image ret(ivec2(width_, height_), 4);

    for (i32 i = 0; i < width_ * height_; ++i)
    {
        ret.data()[i*4] = (byte)255;
        ret.data()[i*4 + 1] = (byte)255;
        ret.data()[i*4 + 2] = (byte)255;
        ret.data()[i*4 + 3] = byte(pixels_[i] * 255.f);
    }

    return ret;
}
