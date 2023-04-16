// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_string.hpp>


namespace dviglo
{

bool set_locale();
std::vector<StrUtf8> main_args_to_vector(i32 argc, char* argv[]);

} // namespace dviglo


#define DV_DEFINE_APP(ClassName)                                            \
    static ClassName* app = nullptr;                                        \
                                                                            \
    SDL_AppResult SDL_AppInit(void** appstate, i32 argc, char* argv[])      \
    {                                                                       \
        (void)appstate;                                                     \
        if (!set_locale())                                                  \
            return SDL_APP_FAILURE;                                         \
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
