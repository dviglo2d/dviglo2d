// Copyright (c) the Dviglo project
// License: MIT

#include "vertex_buffer.hpp"


namespace dviglo
{

constexpr GLsizei calc_vertex_size(VertexAttributes vertex_attributes)
{
    GLsizei ret = 0;

    if (!!(vertex_attributes & VertexAttributes::position))
        ret += 2 * sizeof(f32);

    if (!!(vertex_attributes & VertexAttributes::color))
        ret += sizeof(u32);

    if (!!(vertex_attributes & VertexAttributes::uv))
        ret += 2 * sizeof(f32);

    return ret;
}

VertexBuffer::VertexBuffer()
    : vao_(0)
    , vbo_(0)
    , num_vertices_(0)
    , capacity_(0)
    , vertex_attributes_(VertexAttributes::none)
{
}

VertexBuffer::VertexBuffer(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data)
{
    create(num_vertices, vertex_attributes, usage, data);
}

VertexBuffer::~VertexBuffer()
{
    release();
}

void VertexBuffer::create(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data)
{
    GLsizei stride = calc_vertex_size(vertex_attributes);
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
        attribute_offset += 2 * sizeof(f32);
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
        attribute_offset += 2 * sizeof(f32);
    }

    capacity_ = num_vertices;
    num_vertices_ = data ? num_vertices : 0;
    vertex_attributes_ = vertex_attributes;
}

void VertexBuffer::recreate(GLsizei num_vertices, VertexAttributes vertex_attributes, BufferUsage usage, const void* data)
{
    release();
    create(num_vertices, vertex_attributes, usage, data);
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
    capacity_ = 0;
    vertex_attributes_ = VertexAttributes::none;
}

void VertexBuffer::set_data(GLsizei num_vertices, const void* data)
{
    // TODO: Добавить проверки
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, calc_vertex_size(vertex_attributes_) * num_vertices, data);
    num_vertices_ = num_vertices;
}

void VertexBuffer::bind()
{
    glBindVertexArray(vao_);
}

} // namespace dviglo
