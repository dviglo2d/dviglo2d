// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <dviglo/main/main.h>

#include <SDL.h>


int run()
{
    SDL_Window* window = SDL_CreateWindowWithPosition(
        "Окно закроется через 3 секунды",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
        0
    );

    if (!window)
        return 1;

    SDL_Delay(3000);

    return 0;
}

DV_DEFINE_MAIN(run);
