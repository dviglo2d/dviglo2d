// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"

#include <glad/gl.h>
#include <glm/glm_wrapped.hpp>

#include <utility> // std::exchange()


namespace dviglo
{

class Texture
{
private:
    GLuint gpu_object_name_; ///< Идентификатор объекта OpenGL
    glm::ivec2 size_;

public:
    Texture()
        : gpu_object_name_(0)
        , size_({})
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
        , size_(std::exchange(other.size_, {}))
    {
    }

    Texture& operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            gpu_object_name_ = std::exchange(other.gpu_object_name_, 0);
            size_ = std::exchange(other.size_, {});
        }

        return *this;
    }

    glm::ivec2 size() const { return size_; }
    i32 width() const { return size_.x; }
    i32 height() const { return size_.y; }

    void bind()
    {
        glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    }
};

} // namespace dviglo
