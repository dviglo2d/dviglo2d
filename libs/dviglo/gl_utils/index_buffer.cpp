// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "index_buffer.h"


namespace dviglo
{

IndexBuffer::IndexBuffer()
    : gpu_object_name_(0)
    , num_indices_(0)
{
}

IndexBuffer::IndexBuffer(GLsizei num_indices, const void* data, GLsizeiptr data_size)
{
    glGenBuffers(1, &gpu_object_name_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_object_name_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
    num_indices_ = num_indices;
}

void IndexBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_object_name_);
}

} // namespace dviglo
