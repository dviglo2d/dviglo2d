// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.h"

#include <glad/gl.h>


namespace dviglo
{

class DV_API Texture
{
private:
    /// Идентификатор объекта OpenGL
    GLuint gpu_object_name_;

public:
    Texture()
        : gpu_object_name_(0)
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
    Texture(Texture&&) = default;
    Texture& operator=(Texture&&) = default;

    void bind()
    {
        glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    }
};

} // namespace dviglo
