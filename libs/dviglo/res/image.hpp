// Copyright (c) 2022-present the Dviglo project
// License: MIT

#include "../std_utils/str.hpp"

#include <utility> // std::exchange()


namespace dviglo
{

class Image
{
private:
    i32 width_;
    i32 height_;
    i32 num_components_;
    byte* data_;

public:
    Image(const StrUtf8& path);
    ~Image();

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут иметь ссылку на уничтоженный data_
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе

    Image(Image&& other) noexcept
        : width_(std::exchange(other.width_, 0))
        , height_(std::exchange(other.height_, 0))
        , num_components_(std::exchange(other.num_components_, 0))
        , data_(std::exchange(other.data_, nullptr))
    {
    }

    Image& operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
            num_components_ = std::exchange(other.num_components_, 0);
            data_ = std::exchange(other.data_, nullptr);
        }

        return *this;
    }

    i32 width() const { return width_; }
    i32 height() const { return height_; }
    i32 num_components() const { return num_components_; }
    const byte* data() const { return data_; }
};

} // namespace dviglo
