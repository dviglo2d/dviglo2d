// Copyright (c) the Dviglo project
// License: MIT

#include "application.hpp"

#include "engine_params.hpp"
#include "timer.hpp"

#include <glad/gl.h>

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

SDL_AppResult Application::main_init()
{
    setup();

    log_ = make_unique<Log>(engine_params::log_path);

    if (!SDL_Init(0))
        return SDL_APP_FAILURE;

    os_window_ = make_unique<OsWindow>();
    shader_cache_ = make_unique<ShaderCache>();
    texture_cache_ = make_unique<TextureCache>();
    audio_ = make_unique<Audio>();
    freetype_ = make_unique<FreeType>();

    start();
    new_frame();

    return SDL_APP_CONTINUE;
}

SDL_AppResult Application::main_iterate()
{
#ifdef DV_CTEST
    // При CTest выходим через duration_ секунд после запуска приложения
    if (duration_ && get_ticks_ns() >= s_to_ns(duration_))
        should_exit_ = true;
#endif

    static i64 old_ticks = get_ticks_ns();
    i64 new_ticks = get_ticks_ns();
    i64 ns = new_ticks - old_ticks;
    old_ticks = new_ticks;

    // Если точности get_ticks_ns() не хватает
    if (ns == 0)
    {
        // Ждём полмиллисекунды
        delay_ns(ns_per_ms / 2);
        return SDL_APP_CONTINUE;
    }

    update(ns);
    draw();
    SDL_GL_SwapWindow(DV_OS_WINDOW->window());

    // Убирает лютую желейность управления в Леталке в Windows при включённой вертикале 60 Гц.
    // Похоже NVIDIA буферизует очень много кадров.
    // Если поставить перед swap, то тоже работает.
    // В Linux желейность небольшая и glFinish() на неё если и влияет, то незаметно.
    // Видимо glFinish() делает композитор X11 и не даёт буферизовать много.
    // В бенчмаке в Windows glFinish() роняет FPS c 640 до 440 в релизной сборке MSVC
    // и с 640 до 540 в релизной сборке MinGW.
    // В Linux в релизной сборке GCC - с 610 до 540.
    // Для 2D-игр падение FPS не особо важно.
    // TODO: Попробовать заменить на обычную задержку
    glFinish();

    if (max_fps > 0)
    {
        i64 frame_time_ns = get_ticks_ns() - new_ticks;
        i64 target_frame_time_ns = ns_per_s / max_fps;

        if (frame_time_ns < target_frame_time_ns)
        {
            SDL_DelayNS(target_frame_time_ns - frame_time_ns);

            // delay_ns(...) менее точный, чем SDL_DelayNS(...)
            //delay_ns(target_frame_time_ns - frame_time_ns);
        }
    }

    if (should_exit_)
    {
        return SDL_APP_SUCCESS;
    }
    else
    {
        new_frame();
        return SDL_APP_CONTINUE;
    }
}

SDL_AppResult Application::main_event(SDL_Event* event)
{
    handle_sdl_event(*event);

    if (should_exit_)
        return SDL_APP_SUCCESS;
    else
        return SDL_APP_CONTINUE;
}

} // namespace dviglo
