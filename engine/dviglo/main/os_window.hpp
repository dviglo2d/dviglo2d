// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "config_base.hpp"

#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>


namespace dviglo
{

class OsWindow final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static OsWindow* instance_ = nullptr;

    bool is_invalid_ = false;

    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;

public:
    static OsWindow* instance() { return instance_; }

    OsWindow(const ConfigBase& config);
    ~OsWindow() final;

    bool is_invalid() const { return is_invalid_; }

    SDL_Window* window() const { return window_; }
    SDL_GLContext gl_context() const { return gl_context_; }

    glm::ivec2 get_size_in_pixels() const;
};

} // namespace dviglo

#define DV_OS_WINDOW (dviglo::OsWindow::instance())
