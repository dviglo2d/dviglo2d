// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <glad/gl.h>


namespace dviglo
{

enum class BufferUsage : GLenum
{
    static_draw = GL_STATIC_DRAW,
    dynamic_draw = GL_DYNAMIC_DRAW,
};

} // namespace dviglo
