// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/string.hpp"

#include <glm/glm.hpp>

#include <utility> // std::exchange()


namespace dviglo
{

class Image
{
private:
    glm::ivec2 size_;
    i32 num_components_;
    u8* data_;

public:
    Image();
    ~Image();

    // Копирование
    Image(const Image& other);
    Image& operator=(const Image& other);

    // Перемещение
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    // Пустое изображение
    Image(glm::ivec2 size, i32 num_components, u32 color = 0);
    Image(i32 width, i32 height, i32 num_components, u32 color = 0);

    // Загрузка из файла.
    // При неудаче и use_error_image копирует данные из error_image
    Image(const StrUtf8& file_path, bool use_error_image = false);

    glm::ivec2 size() const { return size_; }
    i32 width() const { return size_.x; }
    i32 height() const { return size_.y; }
    i32 num_components() const { return num_components_; }
    u8* data() const { return data_; }
    u8* pixel_ptr(i32 x, i32 y) { return data_ + (y * size_.x + x) * num_components_; }
    bool is_inside(i32 x, i32 y) const { return x >= 0 && y >= 0 && x < size_.x && y < size_.y; }
    bool empty() const { return data_ == nullptr; }
    void paste(const Image& img, glm::ivec2 pos);
    Image to_rgba(u32 color);
    void save_png(const StrUtf8& path);
    void blur_triangle(i32 radius);
};

// Чёрно-пурпурное шахматное изображение
extern const Image error_image;

} // namespace dviglo
