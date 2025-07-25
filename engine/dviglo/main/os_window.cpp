// Copyright (c) the Dviglo project
// License: MIT

#include "os_window.hpp"

#include "engine_params.hpp"

#include <dv_log.hpp>
#include <glad/gl.h>

using namespace glm;


namespace dviglo
{

OsWindow::OsWindow()
{
    assert(!instance_);

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        Log::writef_error("{} | !SDL_InitSubSystem(SDL_INIT_VIDEO) | {}", DV_FUNC_SIG, SDL_GetError());
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    if (engine_params::msaa_samples > 1)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, engine_params::msaa_samples);
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, engine_params::window_title.c_str());
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_UNDEFINED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_UNDEFINED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, engine_params::window_size.x);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, engine_params::window_size.y);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN,
                           engine_params::window_mode == WindowMode::resizable
                           || engine_params::window_mode == WindowMode::maximized);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_MAXIMIZED_BOOLEAN,
                           engine_params::window_mode == WindowMode::maximized);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN,
                           engine_params::window_mode == WindowMode::fullscreen
                           || engine_params::window_mode == WindowMode::exclusive_fullscreen);
    window_ = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!window_)
    {
        Log::writef_error("{} | !window_ | {}", DV_FUNC_SIG, SDL_GetError());
        return;
    }

    if (engine_params::window_mode == WindowMode::exclusive_fullscreen)
    {
        SDL_DisplayMode mode;
        SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(window_),
                                            engine_params::window_size.x, engine_params::window_size.y,
                                            0.f, false, &mode);
        SDL_SetWindowFullscreenMode(window_, &mode);
    }

    gl_context_ = SDL_GL_CreateContext(window_);

    if (!gl_context_)
    {
        Log::writef_error("{} | !gl_context_ | {}", DV_FUNC_SIG, SDL_GetError());
        return;
    }

    // Настраиваем VSync
    i32 vsync_ret = SDL_GL_SetSwapInterval(engine_params::vsync);

    // Если не получилось включить адаптивную вертикалку, то пробуем включить обычную
    if (vsync_ret < 0 && engine_params::vsync < 0)
    {
        Log::writef_debug("{} | vsync_ret < 0 && engine_params::vsync < 0 | {}", DV_FUNC_SIG, SDL_GetError());
        vsync_ret = SDL_GL_SetSwapInterval(-engine_params::vsync);
    }

    // Вызывает ошибку при запуске через xvfb-run на сервере ГитХаба. Игнорируем
    if (vsync_ret < 0)
        Log::writef_error("{} | vsync_ret < 0 | {}", DV_FUNC_SIG, SDL_GetError());

    // Версия может отличаться от 30003 (например имеет значение 40002 при запуске
    // через Mesa на сервере ГитХаба)
    i32 gl_version = gladLoadGL(static_cast<GLADloadfunc>(SDL_GL_GetProcAddress));

    if (!gl_version)
    {
        Log::writef_error("{} | !gl_version", DV_FUNC_SIG);
        return;
    }

    Log::writef_info("GL_VENDOR: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    Log::writef_info("GL_RENDERER: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    Log::writef_info("GL_VERSION: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    instance_ = this;
    Log::write_debug("OsWindow constructed");
}

OsWindow::~OsWindow()
{
    instance_ = nullptr;

    if (gl_context_)
        SDL_GL_DestroyContext(gl_context_);

    if (window_)
        SDL_DestroyWindow(window_);

    Log::write_debug("OsWindow destructed");
}

ivec2 OsWindow::get_size_in_pixels() const
{
    ivec2 ret;
    SDL_GetWindowSizeInPixels(window_, &ret.x, &ret.y);
    return ret;
}

} // namespace dviglo
