// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.h"

#include <glad/gl.h>


namespace dviglo
{

class DV_API ShaderProgram
{
private:
    /// Идентификатор объекта OpenGL
    GLuint gpu_object_name_ = 0;

public:
    ShaderProgram() = default;

    /// Геометрический шейдер может отсутствовать
    ShaderProgram(const StrUtf8& vertex_shader_path, const StrUtf8& fragment_shader_path,
                  const StrUtf8& geometry_shader_path = StrUtf8());

    ~ShaderProgram()
    {
        glDeleteProgram(gpu_object_name_); // Проверка на 0 не нужна
    }

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе
    ShaderProgram(ShaderProgram&&) = default;
    ShaderProgram& operator=(ShaderProgram&&) = default;

    void use() const
    {
        glUseProgram(gpu_object_name_);
    }
};

} // namespace dviglo
