// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"


namespace dviglo
{

std::vector<StrUtf8> main_args_to_vector(i32 argc, char* argv[]);

} // namespace dviglo


#define DV_DEFINE_APP(ClassName)                                  \
    static ClassName* app = nullptr;                              \
                                                                  \
    i32 SDL_AppInit(void** appstate, i32 argc, char* argv[])      \
    {                                                             \
        app = new ClassName(main_args_to_vector(argc, argv));     \
        return app->main_init();                                  \
    }                                                             \
                                                                  \
    i32 SDL_AppIterate(void* appstate)                            \
    {                                                             \
        return app->main_iterate();                               \
    }                                                             \
                                                                  \
    i32 SDL_AppEvent(void* appstate, const SDL_Event* event)      \
    {                                                             \
        return app->main_event(event);                            \
    }                                                             \
                                                                  \
    void SDL_AppQuit(void* appstate)                              \
    {                                                             \
        app->main_quit();                                         \
        delete app;                                               \
        app = nullptr;                                            \
    }
