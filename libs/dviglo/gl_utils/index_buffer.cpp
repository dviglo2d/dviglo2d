// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "index_buffer.h"

#include <cassert>


namespace dviglo
{

IndexBuffer::IndexBuffer()
    : gpu_object_name_(0)
    , num_indices_(0)
    , type_(0)
{
}

IndexBuffer::IndexBuffer(GLsizei num_indices, GLenum type, const void* data)
{
    assert(type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT);
    GLsizeiptr index_size = (type == GL_UNSIGNED_SHORT) ? 16 : 32;

    glGenBuffers(1, &gpu_object_name_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_object_name_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * index_size, data, GL_STATIC_DRAW);

    num_indices_ = num_indices;
    type_ = type;
}

void IndexBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_object_name_);
}

} // namespace dviglo
