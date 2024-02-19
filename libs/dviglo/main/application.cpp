// Copyright (c) the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "application.hpp"

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

Application::~Application()
{
    SDL_Quit();
}

void Application::handle_sdl_event(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT)
    {
        should_exit_ = true;
    }
    else if (event.type == SDL_EVENT_WINDOW_RESIZED)
    {
        i32 width = event.window.data1;
        i32 height = event.window.data2;
        glViewport(0, 0, width, height);
    }
}

i32 Application::run()
{
    unique_ptr<Log> log = make_unique<Log>(log_path_);
    unique_ptr<ShaderCache> shader_cache = make_unique<ShaderCache>();
    unique_ptr<TextureCache> texture_cache = make_unique<TextureCache>();

    if (SDL_Init(0) < 0)
        return 1;

    setup();

    unique_ptr<OsWindow> os_window = make_unique<OsWindow>();
    unique_ptr<Audio> audio = make_unique<Audio>();

    start();

    u64 old_ticks = SDL_GetTicksNS();

    while (!should_exit_)
    {
#ifdef DV_CTEST
        // При CTest выходим через duration_ секунд после запуска приложения
        if (duration_ && SDL_GetTicks() > duration_ * SDL_MS_PER_SECOND)
            should_exit_ = true;
#endif

        u64 new_ticks = SDL_GetTicksNS();
        u64 ns = new_ticks - old_ticks;
        old_ticks = new_ticks;

        // Если точности SDL_GetTicksNS() не хватает
        if (ns == 0)
        {
            // Ждём полмиллисекунды
            SDL_DelayNS(SDL_NS_PER_MS / 2);
            continue;
        }

        SDL_PumpEvents();
        SDL_Event event;

        while (SDL_PollEvent(&event))
            handle_sdl_event(event);

        update(ns);
        draw();
        SDL_GL_SwapWindow(DV_OS_WINDOW->window());

        //SDL_Delay(500);
    }

    return 0;
}

} // namespace dviglo
