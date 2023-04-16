// Copyright (c) the Dviglo project
// License: MIT

#include "fbo.hpp"

#include <dv_log.hpp>

using namespace glm;
using namespace std;


namespace dviglo
{

Fbo::Fbo(const ivec2 size, const i32 num_components)
{
    glGenFramebuffers(1, &gpu_object_name_);
    glBindFramebuffer(GL_FRAMEBUFFER, gpu_object_name_);

    texture_ = make_unique<Texture>(size, num_components);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_->gpu_object_name(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Log::writef_error("{} | glCheckFramebufferStatus(GL_FRAMEBUFFER) returns {}", DV_FUNC_SIG, glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

} // namespace dviglo
