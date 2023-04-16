// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "texture.hpp"

#include <memory>


namespace dviglo
{

// Framebuffer object
class Fbo
{
private:
    // Идентификатор объекта OpenGL
    GLuint gpu_object_name_ = 0;

    std::unique_ptr<Texture> texture_;

public:
    // В конструкторе меняется текущий FBO
    Fbo(glm::ivec2 size);

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    Fbo(const Fbo&) = delete;
    Fbo& operator=(const Fbo&) = delete;

    ~Fbo()
    {
        glDeleteFramebuffers(1, &gpu_object_name_); // Проверка на 0 не нужна
        gpu_object_name_ = 0;
    }

    Texture* texture() const { return texture_.get(); }
    std::unique_ptr<Texture> move_texture() { return std::move(texture_); }
    GLuint gpu_object_name() const { return gpu_object_name_; }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, gpu_object_name_);
    }
};

} // namespace dviglo
