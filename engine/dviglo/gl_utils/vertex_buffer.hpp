// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "gl_common.hpp"

#include "../common/primitive_types.hpp"
#include "../std_utils/flags.hpp"

#include <utility> // std::exchange()


namespace dviglo
{

enum class VertexAttributes : u32
{
    none     = 0,
    position = 1 << 0,
    color    = 1 << 1, // 0xAABBGGRR
    uv       = 1 << 2,
};
DV_FLAGS(VertexAttributes);


class VertexBuffer
{
private:
    GLuint vao_;
    GLuint vbo_;
    GLsizei num_vertices_; // Число вершин
    GLsizei capacity_; // Максимальное число вершин
    VertexAttributes vertex_attributes_;

    // Если data == nullptr, то выделяет память на GPU без копирования данных
    void create(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data);

public:
    VertexBuffer();

    // Если data == nullptr, то выделяет память на GPU без копирования данных
    VertexBuffer(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data);

    ~VertexBuffer();

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе

    VertexBuffer(VertexBuffer&& other) noexcept
        : vao_(std::exchange(other.vao_, 0))
        , vbo_(std::exchange(other.vbo_, 0))
        , num_vertices_(std::exchange(other.num_vertices_, 0))
        , capacity_(std::exchange(other.capacity_, 0))
        , vertex_attributes_(std::exchange(other.vertex_attributes_, VertexAttributes::none))
    {
    }

    VertexBuffer& operator=(VertexBuffer&& other) noexcept
    {
        if (this != &other)
        {
            vao_ = std::exchange(other.vao_, 0);
            vbo_ = std::exchange(other.vbo_, 0);
            num_vertices_ = std::exchange(other.num_vertices_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            vertex_attributes_ = std::exchange(other.vertex_attributes_, VertexAttributes::none);
        }

        return *this;
    }

    GLsizei num_vertices() const { return num_vertices_; }
    GLsizei capacity() const { return capacity_; }

    // Если data == nullptr, то выделяет память на GPU без копирования данных
    void recreate(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data);

    // Копирует данные в предварительно выделенную на GPU память
    void set_data(GLsizei num_vertices, const void* data);

    void release();
    void bind();
};

} // namespace dviglo
