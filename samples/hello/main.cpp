// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <dviglo/common/primitive_types.h>
#include <dviglo/main/main.h>
#include <dviglo/std_utils/scope_guard.h>

#include <SDL.h>

using namespace dviglo;


i32 run()
{
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

DV_DEFINE_MAIN(run);
