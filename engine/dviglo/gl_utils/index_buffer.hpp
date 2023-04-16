// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "gl_common.hpp"

#include <utility> // std::exchange()


namespace dviglo
{

enum class IndexType : GLenum
{
    u16 = GL_UNSIGNED_SHORT,
    u32 = GL_UNSIGNED_INT,
};


class IndexBuffer
{
private:
    GLuint gpu_object_name_ = 0; // Идентификатор объекта OpenGL
    GLsizei num_indices_;
    GLenum type_;

public:
    IndexBuffer();
    IndexBuffer(GLsizei num_indices, IndexType type, BufferUsage usage, const void* data);

    ~IndexBuffer()
    {
        if (gpu_object_name_)
            glDeleteBuffers(1, &gpu_object_name_);
    }

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе

    IndexBuffer(IndexBuffer&& other) noexcept
        : gpu_object_name_(std::exchange(other.gpu_object_name_, 0))
        , num_indices_(std::exchange(other.num_indices_, 0))
        , type_(std::exchange(other.type_, 0))
    {
    }

    IndexBuffer& operator=(IndexBuffer&& other) noexcept
    {
        if (this != &other)
        {
            gpu_object_name_ = std::exchange(other.gpu_object_name_, 0);
            num_indices_ = std::exchange(other.num_indices_, 0);
            type_ = std::exchange(other.type_, 0);
        }

        return *this;
    }

    GLsizei num_indices() const { return num_indices_; }
    GLenum type() const { return type_; }

    void bind();
};

} // namespace dviglo
