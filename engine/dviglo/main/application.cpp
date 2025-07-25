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
    // Нулевой аргумент - запускаемый файл, поэтому пропускаем
    for (size_t i = 1; i < args.size(); ++i)
    {
        const StrUtf8& arg = args[i];

        if (arg == "-duration")
        {
            // Получаем следующий аргумент, если есть
            if (i + 1 < args.size())
            {
                ++i;
#ifdef DV_CTEST
                duration_ = to_u64(args[i]);
#endif
            }
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

    static i64 old_ticks_ns = get_ticks_ns(); // Время начала прошлого кадра
    i64 new_ticks_ns = get_ticks_ns(); // Время начала кадра
    i64 ns = new_ticks_ns - old_ticks_ns;
    assert(ns >= 0);
    old_ticks_ns = new_ticks_ns;

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

    // Убираем буферизацию. Мы должны знать, когда рендеринг закончился
    glFinish();

    if (max_fps_ > 0)
    {
        // Время update(...) + draw()
        i64 frame_time_ns = get_ticks_ns() - new_ticks_ns;

        // Каким должно быть время кадра, чтобы получить нужный FPS
        i64 target_frame_time_ns = ns_per_s / max_fps_;
        // Лучше ошибиться в сторону большего FPS
        target_frame_time_ns -= 10 * ns_per_us; // -10 микросекунд

        if (frame_time_ns < target_frame_time_ns)
        {
            i64 delay_duration_ns = target_frame_time_ns - frame_time_ns;

            // Точности delay_ns(...) не хватает, поэтому делаем задержку меньше, а потом доводим до нужной
            delay_duration_ns -= 2 * ns_per_ms ; // -2 миллисекунды

            if (delay_duration_ns > 0)
                delay_ns(delay_duration_ns);

            // Тонкая доводка (пустой цикл)
            while ((get_ticks_ns() - new_ticks_ns) < target_frame_time_ns);
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
