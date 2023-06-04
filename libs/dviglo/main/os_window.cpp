// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "os_window.hpp"

#include "engine_params.hpp"

#include "../io/log.hpp"

#include <glad/gl.h>
#include <fmt/format.h>

#include <cassert>

using namespace fmt;


namespace dviglo
{

OsWindow::OsWindow()
{
    assert(!instance_);

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        DV_LOG->write_error(format("OsWindow::OsWindow(): SDL_InitSubSystem(SDL_INIT_VIDEO) < 0 | {}", SDL_GetError()));
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL;

    window_ = SDL_CreateWindowWithPosition(
        engine_params::window_title.c_str(),
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, engine_params::window_size.x, engine_params::window_size.y,
        flags
    );

    if (!window_)
    {
        DV_LOG->write_error(format("Application::run(): !window_ | {}", SDL_GetError()));
        return;
    }

    gl_context_ = SDL_GL_CreateContext(window_);

    if (!gl_context_)
    {
        DV_LOG->write_error(format("Application::run(): !gl_context_ | {}", SDL_GetError()));
        return;
    }

    // Отключаем VSync
    i32 vsync_ret = SDL_GL_SetSwapInterval(0);

    // Вызывает ошибку при запуске через xvfb-run на сервере ГитХаба. Игнорируем
    if (vsync_ret < 0)
        DV_LOG->write_error(format("Application::run(): vsync_ret < 0 | {}", SDL_GetError()));

    // Версия может отличаться от 30003 (например имеет значение 40002 при запуске
    // через Mesa на сервере ГитХаба)
    i32 gl_version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);

    if (!gl_version)
    {
        DV_LOG->write_error("Application::run(): !gl_version");
        return;
    }

    DV_LOG->write_info(format("GL_VENDOR: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR))));
    DV_LOG->write_info(format("GL_RENDERER: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
    DV_LOG->write_info(format("GL_VERSION: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION))));

    instance_ = this;
    DV_LOG->write_debug("OsWindow constructed");
}

OsWindow::~OsWindow()
{
    instance_ = nullptr;

    if (gl_context_)
        SDL_GL_DeleteContext(gl_context_);

    if (window_)
        SDL_DestroyWindow(window_);

    DV_LOG->write_debug("OsWindow destructed");
}

} // namespace dviglo
