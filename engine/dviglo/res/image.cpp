// Copyright (c) the Dviglo project
// License: MIT

#include "image.hpp"

#include "../fs/file_base.hpp"
#include "../fs/log.hpp"
#include "../main/timer.hpp"
#include "../math/rect.hpp"

// Miniz сжимает PNG сильнее и быстрее, чем stb_image_write
#define DV_USE_MINIZ 1

#if DV_USE_MINIZ
    #define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
    #include <miniz.h>
#else
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #define STBIW_WINDOWS_UTF8
    #include "stb_image_write_wrapped.hpp"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

using namespace glm;
using namespace std;


namespace dviglo
{

Image::Image()
    : size_(0, 0)
    , num_components_(0)
    , data_(nullptr)
{
}

Image::Image(ivec2 size, i32 num_components, u32 color)
    : size_(size)
    , num_components_(num_components)
{
    data_ = (u8*)STBI_MALLOC(size_.x * size_.y * num_components_);

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 6387)
#endif

    if (!color)
    {
        memset(data_, 0, size_.x * size_.y * num_components_);
    }
    else
    {
        for (i32 i = 0; i < size_.x * size_.y; ++i)
            memcpy(data_ + i * num_components_, &color, num_components_);
    }

#ifdef _MSC_VER
    #pragma warning(pop)
#endif
}

Image::Image(i32 width, i32 height, i32 num_components, u32 color)
    : Image(ivec2(width, height), num_components, color)
{
}

Image::Image(const Image& other)
{
    size_ = other.size_;
    num_components_ = other.num_components_;
    data_ = (u8*)STBI_MALLOC(size_.x * size_.y * num_components_);

#ifdef _MSC_VER
    #pragma warning(suppress: 6387)
#endif
    memcpy(data_, other.data_, size_.x * size_.y * num_components_);
}

Image& Image::operator=(const Image& other)
{
    if (this != &other)
    {
        stbi_image_free(data_);

        size_ = other.size_;
        num_components_ = other.num_components_;
        data_ = (u8*)STBI_MALLOC(size_.x * size_.y * num_components_);

#ifdef _MSC_VER
    #pragma warning(suppress: 6387)
#endif
        memcpy(data_, other.data_, size_.x * size_.y * num_components_);
    }

    return *this;
}

Image::Image(Image&& other) noexcept
    : size_(std::exchange(other.size_, {}))
    , num_components_(std::exchange(other.num_components_, 0))
    , data_(std::exchange(other.data_, nullptr))
{
}

Image& Image::operator=(Image&& other) noexcept
{
    if (this != &other)
    {
        size_ = std::exchange(other.size_, {});
        num_components_ = std::exchange(other.num_components_, 0);
        data_ = std::exchange(other.data_, nullptr);
    }

    return *this;
}

Image::~Image()
{
    stbi_image_free(data_);
}

Image::Image(const StrUtf8& file_path, bool use_error_image)
{
    data_ = (u8*)stbi_load(file_path.c_str(), &size_.x, &size_.y, &num_components_, 0);

    if (!data_)
    {
        DV_LOG->writef_error("Image::Image(\"{}\"): {}", file_path, stbi_failure_reason());

        if (use_error_image)
            *this = error_image;
    }
}

void Image::save_png(const StrUtf8& path)
{
    i64 begin_time_ms = get_ticks_ms();

#if DV_USE_MINIZ
    i32 compr_level = MZ_DEFAULT_LEVEL;
    size_t png_data_size = 0;
    void* png_data = tdefl_write_image_to_png_file_in_memory_ex(data_, size_.x, size_.y, num_components_, &png_data_size, compr_level, 0);

    if (!png_data)
    {
        DV_LOG->write_error("Image::save_png(const StrUtf8&) | !png_data");
    }
    else
    {
        FILE* stream = file_open(path.c_str(), "wb");
        file_write(png_data, (i32)png_data_size, 1, stream);
        file_close(stream);
        mz_free(png_data);
    }
#else
    stbi_write_png_compression_level = 8;
    stbi_write_png(path.c_str(), size_.x, size_.y, num_components_, data_, 0);
#endif

    i64 duration_ms = get_ticks_ms() - begin_time_ms;
    DV_LOG->writef_info("Image::save_png(const StrUtf8&) | {} | Saved in {} ms", path, duration_ms);
}

void Image::paste(const Image& img, ivec2 pos)
{
    if (img.num_components() != num_components())
    {
        // TODO: Конвертировать вставляемое изображение
        DV_LOG->write_error("Image::paste(): img.num_components() != num_components()");
        return;
    }

    // Границы вставляемого изображения
    IntRect img_rect({0, 0}, img.size());

    if (pos.x < 0)
    {
        img_rect.pos.x -= pos.x;  // Смещаем границы вправо
        img_rect.size.x += pos.x; // Уменьшаем ширину
        pos.x = 0;

        if (img_rect.size.x <= 0) // Вставляемое изображение целиком за левой границей
            return;
    }
    else if (pos.x >= size().x) // Вставляемое изображение целиком за правой границей
    {
        return;
    }

    if (pos.x + img_rect.size.x > size().x) // Вставляемое изображение не умещается
        img_rect.size.x = size().x - pos.x;

    if (pos.y < 0)
    {
        img_rect.pos.y -= pos.y;  // Смещаем границы вниз
        img_rect.size.y += pos.y; // Уменьшаем высоту
        pos.y = 0;

        if (img_rect.size.y <= 0) // Вставляемое изображение целиком за верхней границей
            return;
    }
    else if (pos.y >= size().y) // Вставляемое изображение целиком за нижней границей
    {
        return;
    }

    if (pos.y + img_rect.size.y > size().y) // Вставляемое изображение не умещается
        img_rect.size.y = size().y - pos.y;

    // Копируем линии вставляемого изображения
    for (i32 img_y = img_rect.pos.y, this_y = pos.y;
         img_y < img_rect.pos.y + img_rect.size.y;
         ++img_y, ++this_y)
    {
        i32 img_data_offset = (img_y * img.size().x + img_rect.pos.x) * img.num_components();
        i32 this_data_offset = (this_y * size().x + pos.x) * num_components();
        memcpy(data() + this_data_offset, img.data() + img_data_offset, img_rect.size.x * num_components());
    }
}

Image Image::to_rgba(u32 color)
{
    // TODO: Пока только grayscale изображение
    if (num_components() != 1)
    {
        DV_LOG->write_error("Image::to_rgba(u32) | num_components() != 1");
        return *this;
    }

    Image ret(size_, 4);

    for (i32 i = 0; i < size_.x * size_.y; ++i)
    {
        u32 color_a = (color & 0xFF000000) >> 24;
        u32 a = data_[i] * color_a / 255;
        u32 bgr = color & 0x00FFFFFF;
        u32 abgr = (a << 24) | bgr;
        memcpy(ret.data() + i * 4, &abgr, 4);
    }

    return ret;
}

// TODO: Сделать многопоточным, SIMD или на видеокарте размывать
void Image::blur_triangle(i32 radius)
{
    if (!radius)
        return;

    if (!size_.x || !size_.y)
        return;

    // TODO: проверить, что радиус не слишком большой

    // TODO: Пока только grayscale изображение
    if (num_components() != 1)
    {
        DV_LOG->write_error("Image::blur_triangle(i32) | num_components() != 1");
        return;
    }

    // Пиксель на каждом проходе размывается в две стороны, и получается отрезок
    // длиной radius + 1 + radius.
    // У крайних пикселей вес самый маленький и равен 1.
    // У центрального пикселя вес самый большой и равен radius + 1.
    // Вес соседних пикселей отличается на 1.
    // Сумму весов всех пикселей в этом отрезке можно определить сразу
    i32 total_weight = (radius + 1) * (radius + 1);

    Image tmp(size_, num_components_);

    // Размываем по вертикали и сохраняем результат в tmp
    for (i32 x = 0; x < size_.x; ++x)
    {
        for (i32 y = 0; y < size_.y; ++y)
        {
            // Сразу записываем вклад центрального пикселя.
            // Его вес равен radius + 1
            u32 sum = (u32)pixel_ptr(x, y)[0] * (radius + 1);
            i32 dist = 1;

            while (dist <= radius)
            {
                i32 weight = 1 + radius - dist;

                // Пиксель вне изображения черный, ноль можно не плюсовать.
                // Так что тут все корректно
                if (is_inside(x, y + dist))
                    sum += (u32)pixel_ptr(x, y + dist)[0] * weight;

                if (is_inside(x, y - dist))
                    sum += (u32)pixel_ptr(x, y - dist)[0] * weight;

                ++dist;
            }

            // Сумму нужно поделить на общий вес, иначе изменится яркость изображения
            tmp.pixel_ptr(x, y)[0] = u8(sum / total_weight);
        }
    }

    // Размываем по горизонтали и сохраняем результат назад в структуру.
    for (i32 x = 0; x < size_.x; ++x)
    {
        for (i32 y = 0; y < size_.y; ++y)
        {
            u32 sum = (u32)tmp.pixel_ptr(x, y)[0] * (radius + 1);
            i32 dist = 1;

            while (dist <= radius)
            {
                i32 weight = 1 + radius - dist;

                if (tmp.is_inside(x + dist, y))
                    sum += (u32)tmp.pixel_ptr(x + dist, y)[0] * weight;

                if (tmp.is_inside(x - dist, y))
                    sum += (u32)tmp.pixel_ptr(x - dist, y)[0] * weight;

                ++dist;
            }

            pixel_ptr(x, y)[0] = u8(sum / total_weight);
        }
    }
}

const Image error_image = []
{
    const i32 image_size = 64;
    const i32 num_components = 3; // Можно и 4
    const i32 square_size = 8;

    // Цвета
    const u32 magenta = 0xFFFF00FF;
    const u32 black = 0xFF000000;

    Image ret(image_size, image_size, num_components);

    for (i32 y = 0; y < image_size; ++y)
    {
        for (i32 x = 0; x < image_size; ++x)
        {
            // Номер квадратика по x и y
            i32 square_index_x = x / square_size;
            i32 square_index_y = y / square_size;

            // Оба номера чётные или оба номера нечётные
            u32 color = (square_index_x % 2 == square_index_y % 2) ? black : magenta;

            // Устанавливаем цвет пикселя
            memcpy(ret.data() + y * image_size * num_components + x * num_components, &color, num_components);
        }
    }

    return ret;
}();

} // namespace dviglo
