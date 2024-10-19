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

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут иметь ссылку на уничтоженный data_
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе

    Image(Image&& other) noexcept
        : size_(std::exchange(other.size_, {}))
        , num_components_(std::exchange(other.num_components_, 0))
        , data_(std::exchange(other.data_, nullptr))
    {
    }

    Image& operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            size_ = std::exchange(other.size_, {});
            num_components_ = std::exchange(other.num_components_, 0);
            data_ = std::exchange(other.data_, nullptr);
        }

        return *this;
    }

    glm::ivec2 size() const { return size_; }
    i32 width() const { return size_.x; }
    i32 height() const { return size_.y; }
    i32 num_components() const { return num_components_; }
    const byte* data() const { return data_; }
    bool empty() const { return data_ == nullptr; }

    void save_png(const StrUtf8& path);

    static Image from_file(const StrUtf8& path);
};

} // namespace dviglo
