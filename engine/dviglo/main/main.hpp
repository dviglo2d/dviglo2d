// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/string.hpp"


namespace dviglo
{

std::vector<StrUtf8> main_args_to_vector(i32 argc, char* argv[]);

} // namespace dviglo


#define DV_DEFINE_APP(ClassName)                                            \
    static ClassName* app = nullptr;                                        \
                                                                            \
    SDL_AppResult SDL_AppInit(void** appstate, i32 argc, char* argv[])      \
    {                                                                       \
        (void)appstate;                                                     \
        app = new ClassName(main_args_to_vector(argc, argv));               \
        return app->main_init();                                            \
    }                                                                       \
                                                                            \
    SDL_AppResult SDL_AppIterate(void* appstate)                            \
    {                                                                       \
        (void)appstate;                                                     \
        return app->main_iterate();                                         \
    }                                                                       \
                                                                            \
    SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)            \
    {                                                                       \
        (void)appstate;                                                     \
        return app->main_event(event);                                      \
    }                                                                       \
                                                                            \
    void SDL_AppQuit(void* appstate, SDL_AppResult result)                  \
    {                                                                       \
        (void)appstate;                                                     \
        (void)result;                                                       \
        delete app;                                                         \
        app = nullptr;                                                      \
    }
