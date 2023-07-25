// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"

#include <glad/gl.h>

#include <utility> // std::exchange()


namespace dviglo
{

class Texture
{
private:
    GLuint gpu_object_name_; ///< Идентификатор объекта OpenGL
    i32 width_;
    i32 height_;

public:
    Texture()
        : gpu_object_name_(0)
        , width_(0)
        , height_(0)
    {
    }

    Texture(const StrUtf8& file_path);

    ~Texture()
    {
        glDeleteTextures(1, &gpu_object_name_); // Проверка на 0 не нужна
    }

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе

    Texture(Texture&& other) noexcept
        : gpu_object_name_(std::exchange(other.gpu_object_name_, 0))
        , width_(std::exchange(other.width_, 0))
        , height_(std::exchange(other.height_, 0))
    {
    }

    Texture& operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            gpu_object_name_ = std::exchange(other.gpu_object_name_, 0);
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
        }

        return *this;
    }

    i32 width() const { return width_; }
    i32 height() const { return height_; }

    void bind()
    {
        glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    }
};

} // namespace dviglo
