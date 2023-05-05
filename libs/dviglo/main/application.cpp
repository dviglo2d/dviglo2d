// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "application.h"

#include "../io/log.h"
#include "../std_utils/scope_guard.h"

#include <SDL.h>

#include <memory>

using namespace std;


namespace dviglo
{

Application::Application()
{
}

Application::~Application()
{
    if (window_)
        SDL_DestroyWindow(window_);

    SDL_Quit();
}

i32 Application::run()
{
    unique_ptr<Log> log = make_unique<Log>(log_path_);

    if (SDL_Init(0) < 0)
        return 1;

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
        return 1;

    window_ = SDL_CreateWindowWithPosition(
        "Игра",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
        0
    );

    if (!window_)
        return 1;

    bool should_exit = false;

    while (!should_exit)
    {
        SDL_PumpEvents();
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                should_exit = true;
                break;
            }
        }
    }

    return 0;
}

} // namespace dviglo
