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
}

i32 Application::run()
{
    unique_ptr<Log> log = make_unique<Log>(log_path_);

    // Вызываем SDL_Quit(), даже если SDL_Init() вернул ошибку
    const ScopeGuard sg_sdl_quit = [] { SDL_Quit(); };

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    SDL_Window* window = SDL_CreateWindowWithPosition(
        "Окно закроется через 3 секунды",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
        0
    );

    if (!window)
        return 1;

    const ScopeGuard sg_destroy_window = [window] { SDL_DestroyWindow(window); };

    SDL_Delay(3000);

    return 0;
}

} // namespace dviglo
