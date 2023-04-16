// Copyright (c) the Dviglo project
// License: MIT

#include "fbo.hpp"

#include "../fs/log.hpp"

using namespace glm;
using namespace std;


namespace dviglo
{

Fbo::Fbo(ivec2 size)
{
    glGenFramebuffers(1, &gpu_object_name_);
    glBindFramebuffer(GL_FRAMEBUFFER, gpu_object_name_);

    texture_ = make_unique<Texture>(size);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_->gpu_object_name(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        DV_LOG->writef_error(R"(Fbo::Fbo(ivec2 size) | glCheckFramebufferStatus() returns {})", glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

} // namespace dviglo
