// Copyright (c) the Dviglo project
// License: MIT

#include "gl_utils.hpp"

#include <glad/gl.h>


namespace dviglo
{

IntRect get_viewport()
{
    GLint viewport[4]; // x, y, width, height
    glGetIntegerv(GL_VIEWPORT, viewport);
    return IntRect(viewport[0], viewport[1], viewport[2], viewport[3]);
}

} // namespace dviglo
