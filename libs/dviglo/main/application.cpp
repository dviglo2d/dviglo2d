// Copyright (c) the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "application.hpp"

#include "engine_params.hpp"
#include "os_window.hpp"

#include "../audio/audio.hpp"
#include "../fs/log.hpp"
#include "../gl_utils/shader_cache.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../std_utils/scope_guard.hpp"

#include <glad/gl.h>

#include <memory>

using namespace std;


namespace dviglo
{

Application::Application(const vector<StrUtf8>& args)
    : args_(args)
{
    // Первый аргумент - запускаемый файл, поэтому пропускаем
    for (size_t i = 1; i < args.size(); ++i)
    {
        // Ищем аргумент, который начинается с '-'
        if (args[i].length() > 1 && args[i][0] == '-')
        {
            // Убираем '-' в начале строки
            StrUtf8 argument = args[i].substr(1);

            // Сразу же запоминаем следующий аргумент, если есть
            StrUtf8 value = i + 1 < args.size() ? args[i + 1] : StrUtf8();

#ifdef DV_CTEST
            if (argument == "duration" && !value.empty())
            {
                duration_ = to_u64(value);
                ++i;
            }
#endif
        }
    }
}

void Application::handle_sdl_event(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_QUIT:
        should_exit_ = true;
        return;

    case SDL_EVENT_WINDOW_RESIZED:
        {
            i32 width = event.window.data1;
            i32 height = event.window.data2;
            glViewport(0, 0, width, height);
            return;
        }
    }
}

static Log* log = nullptr;
static ShaderCache* shader_cache = nullptr;
static TextureCache* texture_cache = nullptr;
static OsWindow* os_window = nullptr;
static Audio* audio = nullptr;

SDL_AppResult Application::main_init()
{
    setup();

    log = new Log(engine_params::log_path);
    shader_cache = new ShaderCache();
    texture_cache = new TextureCache();

    if (!SDL_Init(0))
        return SDL_APP_FAILURE;

    os_window = new OsWindow();
    audio = new Audio();

    start();

    return SDL_APP_CONTINUE;
}

SDL_AppResult Application::main_iterate()
{
#ifdef DV_CTEST
    // При CTest выходим через duration_ секунд после запуска приложения
    if (duration_ && SDL_GetTicks() > duration_ * SDL_MS_PER_SECOND)
        should_exit_ = true;
#endif

    static u64 old_ticks = SDL_GetTicksNS();
    u64 new_ticks = SDL_GetTicksNS();
    u64 ns = new_ticks - old_ticks;
    old_ticks = new_ticks;

    // Если точности SDL_GetTicksNS() не хватает
    if (ns == 0)
    {
        // Ждём полмиллисекунды
        SDL_DelayNS(SDL_NS_PER_MS / 2);
        return SDL_APP_CONTINUE;
    }

    update(ns);
    draw();
    SDL_GL_SwapWindow(DV_OS_WINDOW->window());

    if (should_exit_)
        return SDL_APP_SUCCESS;
    else
        return SDL_APP_CONTINUE;
}

SDL_AppResult Application::main_event(SDL_Event* event)
{
    handle_sdl_event(*event);

    if (should_exit_)
        return SDL_APP_SUCCESS;
    else
        return SDL_APP_CONTINUE;
}

void Application::main_quit([[maybe_unused]] SDL_AppResult result)
{
    delete audio;
    audio = nullptr;

    delete os_window;
    os_window = nullptr;

    delete texture_cache;
    texture_cache = nullptr;

    delete shader_cache;
    shader_cache = nullptr;

    delete log;
    log = nullptr;
}

} // namespace dviglo
