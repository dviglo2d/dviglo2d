// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem_base.hpp>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>


namespace dviglo
{

class OsWindow final : public SubsystemBase<OsWindow>
{
private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;

public:
    OsWindow();
    ~OsWindow();

    SDL_Window* window() const { return window_; }
    SDL_GLContext gl_context() const { return gl_context_; }

    glm::ivec2 get_size_in_pixels() const;
};

} // namespace dviglo

#define DV_OS_WINDOW (dviglo::OsWindow::instance())
