// Copyright (c) the Dviglo project
// License: MIT

#include "image.hpp"

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
    #include <stb_image_write.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include <dv_log.hpp>

#include <fstream>
#include <utility> // std::exchange()

using namespace glm;
using namespace std;


#if DV_WINDOWS_MSVC
    // Чтобы не проверять, что malloc(...) возвращает не nullptr
    #pragma warning(disable: 6387)
#endif

namespace dviglo
{

Image::Image()
    : size_(ivec2(0, 0))
    , num_components_(0)
    , data_(nullptr)
{
}

Image::Image(ivec2 size, i32 num_components, u32 color, bool warning)
{
    size_t num_bytes = size.x * size.y * num_components;

    if (!num_bytes)
    {
        size_ = ivec2(0, 0);
        num_components_ = 0;
        data_ = nullptr;

        if (warning)
            Log::writef_warning("{} | !num_bytes", DV_FUNC_SIG);
    }
    else
    {
        size_ = size;
        num_components_ = num_components;
        data_ = static_cast<u8*>(STBI_MALLOC(num_bytes));

        if (!color)
        {
            memset(data_, 0, num_bytes);
        }
        else
        {
            for (i32 i = 0; i < size_.x * size_.y; ++i)
                memcpy(data_ + i * num_components_, &color, num_components_);
        }
    }
}

Image::Image(i32 width, i32 height, i32 num_components, u32 color, bool warning)
    : Image(ivec2(width, height), num_components, color, warning)
{
}

Image::Image(const Image& other)
{
    size_t num_bytes = other.size_.x * other.size_.y * other.num_components_;

    if (!num_bytes)
    {
        size_ = ivec2(0, 0);
        num_components_ = 0;
        data_ = nullptr;
    }
    else
    {
        size_ = other.size_;
        num_components_ = other.num_components_;
        data_ = static_cast<u8*>(STBI_MALLOC(num_bytes));
        memcpy(data_, other.data_, num_bytes);
    }
}

Image& Image::operator =(const Image& other)
{
    if (this != &other)
    {
        stbi_image_free(data_);

        size_ = other.size_;
        num_components_ = other.num_components_;

        size_t num_bytes = size_.x * size_.y * num_components_;

        if (!num_bytes)
        {
            data_ = nullptr;
        }
        else
        {

            data_ = static_cast<u8*>(STBI_MALLOC(num_bytes));
            memcpy(data_, other.data_, num_bytes);
        }
    }

    return *this;
}

Image::Image(Image&& other) noexcept
    : size_(std::exchange(other.size_, {}))
    , num_components_(std::exchange(other.num_components_, 0))
    , data_(std::exchange(other.data_, nullptr))
{
}

Image& Image::operator =(Image&& other) noexcept
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
    size_ = ivec2(0, 0);
    num_components_ = 0;
    stbi_image_free(data_);
    data_ = nullptr;
}

Image::Image(const fs::path& path, bool use_error_image)
{
    // В случае неудачи не меняет size_ и num_components_
    data_ = stbi_load(path.string().c_str(), &size_.x, &size_.y, &num_components_, 0);

    if (!data_)
    {
        Log::writef_error("{} | {} | {}", DV_FUNC_SIG, path, stbi_failure_reason());

        if (use_error_image)
        {
            *this = error_image_rgba;
        }
        else
        {
            size_ = ivec2(0, 0);
            num_components_ = 0;
        }
    }
}

void Image::save_png(const fs::path& path) const
{
    i64 begin_time_ms = get_ticks_ms();

#if DV_USE_MINIZ
    i32 compr_level = MZ_DEFAULT_LEVEL;
    size_t png_data_size = 0;
    void* png_data = tdefl_write_image_to_png_file_in_memory_ex(data_, size_.x, size_.y, num_components_, &png_data_size, compr_level, 0);

    if (!png_data)
    {
        Log::writef_error("{} | {} | !png_data", DV_FUNC_SIG, path);
    }
    else
    {
        ofstream stream(path, ios::binary);
        stream.write(reinterpret_cast<const char*>(png_data), (i32)png_data_size);
        mz_free(png_data);
    }
#else
    stbi_write_png_compression_level = 8;
    stbi_write_png(path.string().c_str(), size_.x, size_.y, num_components_, data_, 0);
#endif

    i64 duration_ms = get_ticks_ms() - begin_time_ms;
    Log::writef_info("{} | {} | Saved in {} ms", DV_FUNC_SIG, path, duration_ms);
}

void Image::paste(const Image& img, ivec2 pos)
{
    if (empty() || img.empty())
        return;

    if (img.num_components() != num_components())
    {
        // TODO: Конвертировать вставляемое изображение
        Log::writef_error("{} | img.num_components() != num_components()", DV_FUNC_SIG);
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
    if (empty())
        return Image();

    // TODO: Пока только grayscale изображение
    if (num_components() != 1)
    {
        Log::writef_error("{} | num_components() != 1", DV_FUNC_SIG);
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

// TODO: Может быть SIMD использовать
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
        Log::writef_error("{} | num_components() != 1", DV_FUNC_SIG);
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
    #pragma omp parallel for // Распараллеливание внешнего цикла с помощью OpenMP
    for (i32 x = 0; x < size_.x; ++x)
    {
        for (i32 y = 0; y < size_.y; ++y)
        {
            // Сразу записываем вклад центрального пикселя.
            // Его вес равен radius + 1
            u32 sum = (u32)pixel_ptr(x, y)[0] * (radius + 1);

            for (i32 dist = 1; dist <= radius; ++dist)
            {
                i32 weight = 1 + radius - dist;

                // Пиксель вне изображения черный, ноль можно не плюсовать.
                // Так что тут все корректно
                if (is_inside(x, y + dist))
                    sum += (u32)pixel_ptr(x, y + dist)[0] * weight;

                if (is_inside(x, y - dist))
                    sum += (u32)pixel_ptr(x, y - dist)[0] * weight;
            }

            // Сумму нужно поделить на общий вес, иначе изменится яркость изображения
            tmp.pixel_ptr(x, y)[0] = u8(sum / total_weight);
        }
    }

    // Размываем по горизонтали и сохраняем результат назад в структуру
    #pragma omp parallel for // Распараллеливание внешнего цикла с помощью OpenMP
    for (i32 x = 0; x < size_.x; ++x)
    {
        for (i32 y = 0; y < size_.y; ++y)
        {
            u32 sum = (u32)tmp.pixel_ptr(x, y)[0] * (radius + 1);

            for (i32 dist = 1; dist <= radius; ++dist)
            {
                i32 weight = 1 + radius - dist;

                if (tmp.is_inside(x + dist, y))
                    sum += (u32)tmp.pixel_ptr(x + dist, y)[0] * weight;

                if (tmp.is_inside(x - dist, y))
                    sum += (u32)tmp.pixel_ptr(x - dist, y)[0] * weight;
            }

            pixel_ptr(x, y)[0] = u8(sum / total_weight);
        }
    }
}

static Image generate_error_image(i32 num_components)
{
    assert(num_components >= 1 && num_components <= 4);

    const i32 image_size = 64;
    const i32 square_size = 8;

    // Цвета
    const u32 light = (num_components == 3 || num_components == 4) ? 0xFFFF00FF : 0xFFFFFFFF;
    const u32 dark = 0xFF000000;

    Image ret(image_size, image_size, num_components);

    for (i32 y = 0; y < image_size; ++y)
    {
        for (i32 x = 0; x < image_size; ++x)
        {
            // Номер квадратика по x и y
            i32 square_index_x = x / square_size;
            i32 square_index_y = y / square_size;

            // Оба номера чётные или оба номера нечётные
            u32 color = (square_index_x % 2 == square_index_y % 2) ? dark : light;

            // Устанавливаем цвет пикселя
            memcpy(ret.data() + y * image_size * num_components + x * num_components, &color, num_components);
        }
    }

    return ret;
}

const Image error_image_r = generate_error_image(1);
const Image error_image_rg = generate_error_image(2);
const Image error_image_rgb = generate_error_image(3);
const Image error_image_rgba = generate_error_image(4);

} // namespace dviglo
