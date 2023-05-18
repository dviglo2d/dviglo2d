// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "vertex_buffer.h"

#include "../common/primitive_types.h"


namespace dviglo
{

VertexBuffer::VertexBuffer()
{
    release();
}

VertexBuffer::VertexBuffer(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data)
{
    set_data(num_vertices, vertex_attributes, usage, data);
}

VertexBuffer::~VertexBuffer()
{
    release();
}

void VertexBuffer::set_data(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data)
{
    release();

    // Вычисляем размер одной вершины

    GLsizei stride = 0;

    if (!!(vertex_attributes & VertexAttributes::position))
        stride += 2 * sizeof(float);

    if (!!(vertex_attributes & VertexAttributes::color))
        stride += sizeof(u32);

    if (!!(vertex_attributes & VertexAttributes::uv))
        stride += 2 * sizeof(float);

    // Размер данных
    GLsizeiptr data_size = stride * num_vertices;

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, data_size, data, (GLenum)usage);

    GLuint attribute_index = 0;
    size_t attribute_offset = 0; // Смещение до атрибута вершины от начала вершины

    if (!!(vertex_attributes & VertexAttributes::position))
    {
        glVertexAttribPointer(attribute_index, 2, GL_FLOAT, GL_FALSE, stride, (void*)attribute_offset);
        glEnableVertexAttribArray(attribute_index);

        // Вычисляем индекс и смещение следующего атрибута
        ++attribute_index;
        attribute_offset += 2 * sizeof(float);
    }

    if (!!(vertex_attributes & VertexAttributes::color))
    {
        // На стороне CPU цвет - u32 (0xAABBGGRR), который интерпретируется как четыре отдельных байта.
        // На стороне GPU цвет автоматически преобразуется в четыре float
        glVertexAttribPointer(attribute_index, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)attribute_offset);
        glEnableVertexAttribArray(attribute_index);

        ++attribute_index;
        attribute_offset += sizeof(u32);
    }

    if (!!(vertex_attributes & VertexAttributes::uv))
    {
        glVertexAttribPointer(attribute_index, 2, GL_FLOAT, GL_FALSE, stride, (void*)attribute_offset);
        glEnableVertexAttribArray(attribute_index);

        ++attribute_index;
        attribute_offset += 2 * sizeof(float);
    }

    num_vertices_ = num_vertices;
    vertex_attributes_ = vertex_attributes;
}

void VertexBuffer::release()
{
    if (vbo_)
    {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }

    if (vao_)
    {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }

    num_vertices_ = 0;
    vertex_attributes_ = VertexAttributes::none;
}

void VertexBuffer::bind()
{
    glBindVertexArray(vao_);
}

} // namespace dviglo
