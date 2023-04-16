// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_primitive_types.hpp>
#include <dv_string.hpp>
#include <SDL3/SDL.h>


namespace dviglo
{

// Если == 0.0, то без ограничений.
// Значение можно менять в любое время, от неё зависит задержка в конце кадра
inline void max_fps(f64 value)
{
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, std::to_string(value).c_str());
}

// Если == 0.0, то без ограничений
inline f64 max_fps()
{
    return SDL_atof(SDL_GetHint(SDL_HINT_MAIN_CALLBACK_RATE));
}

} // namespace dviglo
