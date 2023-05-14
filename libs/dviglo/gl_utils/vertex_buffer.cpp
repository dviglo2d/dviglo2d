// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "vertex_buffer.h"


namespace dviglo
{

VertexBuffer::VertexBuffer()
    : gpu_object_name_(0)
    , num_vertices_(0)
{
}

VertexBuffer::VertexBuffer(GLsizei num_vertices, const void* data, GLsizeiptr data_size)
{
    glGenBuffers(1, &gpu_object_name_);
    glBindBuffer(GL_ARRAY_BUFFER, gpu_object_name_);
    glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
    num_vertices_ = num_vertices;
}

void VertexBuffer::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, gpu_object_name_);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

} // namespace dviglo
