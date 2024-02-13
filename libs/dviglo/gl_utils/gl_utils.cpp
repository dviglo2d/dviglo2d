// Copyright (c) the Dviglo project
// License: MIT

#include "gl_utils.hpp"

#include <glad/gl.h>

using namespace glm;


namespace dviglo
{

ivec2 get_viewport_size()
{
    GLint viewport[4]; // x, y, width, height
    glGetIntegerv(GL_VIEWPORT, viewport);
    return {viewport[2], viewport[3]};
}

} // namespace dviglo
