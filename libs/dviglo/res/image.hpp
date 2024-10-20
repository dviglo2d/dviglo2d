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
    byte* data_;

public:
    Image();
    ~Image();

    /// Выделяет память
    Image(glm::ivec2 size, i32 num_components);

    Image(const Image& other);
    Image& operator=(const Image& other);

    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    glm::ivec2 size() const { return size_; }
    i32 width() const { return size_.x; }
    i32 height() const { return size_.y; }
    i32 num_components() const { return num_components_; }
    byte* data() const { return data_; }
    bool empty() const { return data_ == nullptr; }

    void save_png(const StrUtf8& path);

    static Image from_file(const StrUtf8& path);
};

} // namespace dviglo
