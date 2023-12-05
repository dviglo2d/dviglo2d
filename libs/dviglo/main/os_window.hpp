// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <glm/glm_wrapped.hpp>
#include <SDL3/SDL.h>


namespace dviglo
{

class OsWindow
{
private:
    /// Инициализируется в конструкторе
    inline static OsWindow* instance_ = nullptr;

    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;

    /// Размер окна в пикселях может отличаться от размера в экранных координатах
    glm::ivec2 size_in_pixels_{0, 0};

public:
    static OsWindow* instance() { return instance_; }

    OsWindow();
    ~OsWindow();

    SDL_Window* window() const { return window_; }
    SDL_GLContext gl_context() const { return gl_context_; }

    /// Размер окна в пикселях может отличаться от размера в экранных координатах
    glm::ivec2 size_in_pixels() const { return size_in_pixels_; }
};

#define DV_OS_WINDOW (dviglo::OsWindow::instance())

} // namespace dviglo
