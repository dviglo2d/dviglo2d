// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "gl_common.h"

#include "../std_utils/flags.h"


namespace dviglo
{

enum class VertexAttributes
{
    none     = 0,
    position = 1 << 0,
    color    = 1 << 1, // 0xAABBGGRR
    uv       = 1 << 2,
};
DV_FLAGS(VertexAttributes);


class DV_API VertexBuffer
{
private:
    GLuint vao_;
    GLuint vbo_;
    GLsizei num_vertices_;
    VertexAttributes vertex_attributes_;

public:
    VertexBuffer();
    VertexBuffer(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data);

    ~VertexBuffer();

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе
    VertexBuffer(VertexBuffer&&) = default;
    VertexBuffer& operator=(VertexBuffer&&) = default;

    GLsizei num_vertices() const { return num_vertices_; }

    /// Если data == nullptr, то выделяет память на GPU без копирования данных
    void recreate(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data);

    void release();
    void bind();
};

} // namespace dviglo
