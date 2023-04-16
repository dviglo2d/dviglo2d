// Copyright (c) the Dviglo project
// License: MIT

#include "index_buffer.hpp"

#include <cassert>


namespace dviglo
{

IndexBuffer::IndexBuffer()
    : gpu_object_name_(0)
    , num_indices_(0)
    , type_(0)
{
}

IndexBuffer::IndexBuffer(GLsizei num_indices, IndexType type, BufferUsage usage, const void* data)
{
    glGenBuffers(1, &gpu_object_name_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_object_name_);

    GLsizeiptr index_size = (type == IndexType::u16) ? 2 : 4;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * index_size, data, (GLenum)usage);

    num_indices_ = num_indices;
    type_ = (GLenum)type;
}

void IndexBuffer::bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_object_name_);
}

} // namespace dviglo
