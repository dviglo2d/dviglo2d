// Copyright (c) the Dviglo project
// License: MIT

#include "image.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include "stb_image_write_wrapped.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

#include "../math/rect.hpp"

using namespace glm;


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
    data_ = reinterpret_cast<byte*>(STBI_MALLOC(size_.x * size_.y * num_components_));
}

Image::Image(const Image& other)
{
    size_ = other.size_;
    num_components_ = other.num_components_;
    data_ = reinterpret_cast<byte*>(STBI_MALLOC(size_.x * size_.y * num_components_));
    memcpy(data_, other.data_, size_.x * size_.y * num_components_);
}

Image& Image::operator=(const Image& other)
{
    if (this != &other)
    {
        stbi_image_free(data_);

        size_ = other.size_;
        num_components_ = other.num_components_;
        data_ = reinterpret_cast<byte*>(STBI_MALLOC(size_.x * size_.y * num_components_));
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

void Image::paste(const Image& img, ivec2 pos)
{
    if (img.num_components() != num_components())
    {
        // TODO: Конвертировать вставляемое изображение
        //asIScriptContext* ctx = asGetActiveContext();
        //if (ctx)
        //    ctx->SetException("ASImage::paste(): img.num_components() != num_components()");
        return;
    }

    // Границы вставляемого изображения
    IntRect img_rect({0, 0}, img.size());

    if (pos.x < 0)
    {
        img_rect.pos.x -= pos.x;  // Смещаем вправо
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
        img_rect.pos.y -= pos.y;  // Смещаем вниз
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

void Image::save_png(const StrUtf8& path)
{
    stbi_write_png(path.c_str(), size_.x, size_.y, num_components_, data_, 0);
}

Image Image::from_file(const StrUtf8& path)
{
    Image ret;
    ret.data_ = reinterpret_cast<byte*>(stbi_load(path.c_str(), &ret.size_.x, &ret.size_.y, &ret.num_components_, 0));
    return ret;
}

} // namespace dviglo
