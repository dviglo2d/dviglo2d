// Copyright (c) the Dviglo project
// License: MIT

#include "timer.hpp"

#define DV_USE_SDL 1

#if DV_USE_SDL
    #include <SDL3/SDL.h>
    #include <cassert>
#else
    #include <chrono>
    #include <thread>
    using namespace std;
#endif


namespace dviglo
{

#if !DV_USE_SDL
static const auto start_time = chrono::steady_clock::now();
#endif

i64 get_ticks_ms()
{
#if DV_USE_SDL
    i64 ret = static_cast<i64>(SDL_GetTicks());
    assert(ret >= 0);
    return ret;
#else
    auto duration = chrono::steady_clock::now() - start_time;
    return duration_cast<chrono::milliseconds>(duration).count();
#endif
}

i64 get_ticks_ns()
{
#if DV_USE_SDL
    i64 ret = static_cast<i64>(SDL_GetTicksNS());
    assert(ret >= 0);
    return ret;
#else
    auto duration = chrono::steady_clock::now() - start_time;
    return duration_cast<chrono::nanoseconds>(duration).count();
#endif
}

void delay_ms(i64 ms)
{
#if DV_USE_SDL // SDL версия болеее точная
    assert(ms >= 0 && ms <= SDL_MAX_UINT32);
    SDL_Delay(static_cast<Uint32>(ms));
#else
    auto duration = chrono::milliseconds(ms);
    this_thread::sleep_for(duration);
#endif
}

void delay_ns(i64 ns)
{
#if DV_USE_SDL // SDL версия болеее точная
    assert(ns >= 0);
    SDL_DelayNS(static_cast<Uint64>(ns));
#else
    auto duration = chrono::nanoseconds(ns);
    this_thread::sleep_for(duration);
#endif
}

} // namespace dviglo
