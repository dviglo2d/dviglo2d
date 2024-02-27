// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"


namespace dviglo
{

std::vector<StrUtf8> get_command_line_args(i32 argc, char* argv[]);

} // namespace dviglo


#define DV_DEFINE_APP(ClassName)                                  \
    static ClassName* app = nullptr;                              \
                                                                  \
    i32 SDL_AppInit(i32 argc, char* argv[])                       \
    {                                                             \
        app = new ClassName(get_command_line_args(argc, argv));   \
        return app->sdl_init();                                   \
    }                                                             \
                                                                  \
    i32 SDL_AppIterate()                                          \
    {                                                             \
        return app->sdl_iterate();                                \
    }                                                             \
                                                                  \
    i32 SDL_AppEvent(const SDL_Event* event)                      \
    {                                                             \
        return app->sdl_event(event);                             \
    }                                                             \
                                                                  \
    void SDL_AppQuit()                                            \
    {                                                             \
        app->sdl_quit();                                          \
        delete app;                                               \
        app = nullptr;                                            \
    }
