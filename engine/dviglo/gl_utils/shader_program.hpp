// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/string.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <utility> // std::exchange()


namespace dviglo
{

class ShaderProgram
{
private:
    // Идентификатор объекта OpenGL
    GLuint gpu_object_name_ = 0;

public:
    ShaderProgram() = default;

    // Геометрический шейдер может отсутствовать
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

    ShaderProgram(ShaderProgram&& other) noexcept
        : gpu_object_name_(std::exchange(other.gpu_object_name_, 0))
    {
    }

    ShaderProgram& operator=(ShaderProgram&& other) noexcept
    {
        if (this != &other)
            gpu_object_name_ = std::exchange(other.gpu_object_name_, 0);

        return *this;
    }

    void use() const
    {
        glUseProgram(gpu_object_name_);
    }

    void set(const StrAscii& name, GLint value) const
    {
        GLint location = glGetUniformLocation(gpu_object_name_, name.c_str());
        glUniform1i(location, value);
    }

    void set(const StrAscii& name, bool value) const
    {
        GLint location = glGetUniformLocation(gpu_object_name_, name.c_str());
        glUniform1i(location, value);
    }

    void set(const StrAscii& name, glm::vec2 value) const
    {
        GLint location = glGetUniformLocation(gpu_object_name_, name.c_str());
        glUniform2fv(location, 1, &value[0]);
    }

    void set(const StrAscii& name, const glm::vec4& value) const
    {
        GLint location = glGetUniformLocation(gpu_object_name_, name.c_str());
        glUniform4fv(location, 1, &value[0]);
    }
};

} // namespace dviglo
