// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "application.h"

#include "../io/log.h"
#include "../std_utils/scope_guard.h"

#include <GL/glew.h>

#include <format>
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
    if (gl_context_)
        SDL_GL_DeleteContext(gl_context_);

    if (window_)
        SDL_DestroyWindow(window_);

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

    if (SDL_Init(0) < 0)
        return 1;

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
        return 1;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL;

    window_ = SDL_CreateWindowWithPosition(
        "Игра",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
        flags
    );

    if (!window_)
    {
        DV_LOG->write_error(format("Application::run(): !window_ | {}", SDL_GetError()));
        return 1;
    }

    gl_context_ = SDL_GL_CreateContext(window_);

    if (!gl_context_)
    {
        DV_LOG->write_error(format("Application::run(): !gl_context_ | {}", SDL_GetError()));
        return 1;
    }

    GLenum glew_result = glewInit();

    if (glew_result != GLEW_OK)
    {
        DV_LOG->write_error(format("Application::run(): glew_result != GLEW_OK | {}",
                            reinterpret_cast<const char*>(glewGetErrorString(glew_result))));
        return 1;
    }

    DV_LOG->write_info(format("GL_VENDOR: {}", (const char*)glGetString(GL_VENDOR)));
    DV_LOG->write_info(format("GL_RENDERER: {}", (const char*)glGetString(GL_RENDERER)));
    DV_LOG->write_info(format("GL_VERSION: {}", (const char*)glGetString(GL_VERSION)));

    start();

    u64 old_ticks = SDL_GetTicks();

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

        u64 new_ticks = SDL_GetTicks();
        u64 ms = new_ticks - old_ticks;
        old_ticks = new_ticks;

        update(ms);
        draw();
        SDL_GL_SwapWindow(window_);

        SDL_Delay(500);
    }

    return 0;
}

} // namespace dviglo
