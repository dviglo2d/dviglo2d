// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "application.hpp"

#include "os_window.hpp"

#include "../gl_utils/shader_cache.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../io/log.hpp"
#include "../std_utils/scope_guard.hpp"

#include <fmt/format.h>
#include <glad/gl.h>

#include <memory>

using namespace fmt;
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

void Application::on_key(const SDL_KeyboardEvent& event_data)
{
    if (event_data.type == SDL_EVENT_KEY_DOWN && event_data.repeat == false
        && event_data.keysym.scancode == SDL_SCANCODE_ESCAPE)
    {
        should_exit_ = true;
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

    start();

    u64 old_ticks = SDL_GetTicksNS();

    while (!should_exit_)
    {
#ifdef DV_CTEST
        // При CTest выходим через duration_ секунд после запуска приложения
        if (duration_ && SDL_GetTicks() > duration_ * 1000)
            should_exit_ = true;
#endif

        SDL_PumpEvents();
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                should_exit_ = true;
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                on_key(event.key);
                break;
            }
        }

        u64 new_ticks = SDL_GetTicksNS();
        u64 ns = new_ticks - old_ticks;
        old_ticks = new_ticks;

        // Если точности SDL_GetTicksNS() не хватает
        if (ns == 0)
        {
            // Ждём полмиллисекунды
            SDL_DelayNS(500'000);
            continue;
        }

        update(ns);
        draw();
        SDL_GL_SwapWindow(DV_OS_WINDOW->window());

        //SDL_Delay(500);
    }

    return 0;
}

} // namespace dviglo
