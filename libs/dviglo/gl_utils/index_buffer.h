// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../common/config.h"

#include <glad/gl.h>


namespace dviglo
{

class DV_API IndexBuffer
{
private:
    /// Идентификатор объекта OpenGL
    GLuint gpu_object_name_ = 0;

    GLsizei num_indices_;
    GLenum type_;

public:
    IndexBuffer();
    IndexBuffer(GLsizei num_indices, GLenum type, const void* data, GLsizeiptr data_size);

    ~IndexBuffer()
    {
        if (gpu_object_name_)
            glDeleteBuffers(1, &gpu_object_name_);
    }

    GLsizei num_indices() const { return num_indices_; }
    GLenum type() const { return type_; }

    void bind();
};

} // namespace dviglo
