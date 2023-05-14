// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../common/config.h"

#include <glad/gl.h>


namespace dviglo
{

class DV_API VertexBuffer
{
private:
    /// Идентификатор объекта OpenGL
    GLuint gpu_object_name_ = 0;

    /// Число вершин
    GLsizei num_vertices_;

public:
    VertexBuffer();

    VertexBuffer(GLsizei num_vertices, const void* data, GLsizeiptr data_size);

    ~VertexBuffer()
    {
        if (gpu_object_name_)
            glDeleteBuffers(1, &gpu_object_name_);
    }

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе
    VertexBuffer(VertexBuffer&&) = default;
    VertexBuffer& operator=(VertexBuffer&&) = default;

    GLsizei num_vertices() const { return num_vertices_; }

    void bind();
};

} // namespace dviglo
