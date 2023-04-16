// Copyright (c) the Dviglo project
// License: MIT

#include "image.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include "stb_image_write_wrapped.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include "../fs/log.hpp"

#include <format>

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

Image::Image(ivec2 size, i32 num_components)
    : size_(size)
    , num_components_(num_components)
{
    data_ = (u8*)STBI_MALLOC(size_.x * size_.y * num_components_);

#ifdef _MSC_VER
    #pragma warning(suppress: 6387)
#endif
    memset(data_, 0, size_.x * size_.y * num_components_);
}

Image::Image(i32 width, i32 height, i32 num_components)
    : Image(ivec2(width, height), num_components)
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

void Image::save_png(const StrUtf8& path)
{
    stbi_write_png(path.c_str(), size_.x, size_.y, num_components_, data_, 0);
}

Image Image::from_file(const StrUtf8& path)
{
    Image ret;
    ret.data_ = (u8*)stbi_load(path.c_str(), &ret.size_.x, &ret.size_.y, &ret.num_components_, 0);

    if (!ret.data_)
    {
        DV_LOG->write_error(format("Image::from_file(\"{}\"): {}", path, stbi_failure_reason()));
        return error_image;
    }

    return ret;
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
