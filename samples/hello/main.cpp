// Copyright (c) the Dviglo project
// License: MIT

#include "app.hpp"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

namespace dviglo
{

#ifdef _WIN32
    std::vector<StrUtf8> get_win_command_line_args();
#endif

} // namespace dviglo


App* app = nullptr;

int SDL_AppInit(int argc, char** argv)
{
#ifdef DV_WIN32_CONSOLE
    std::setlocale(LC_CTYPE, "en_US.UTF-8");
#endif

    app = new App(get_win_command_line_args());
    app->run();
    return 0;
}

int SDL_AppIterate(void)
{
    app->iterate();
    return 0;
}

int SDL_AppEvent(const SDL_Event* event)
{
    app->handle_sdl_event(*event);

    if (app->should_exit_)
        return 1;
    else
        return 0;
}

void SDL_AppQuit(void)
{
    delete app;
}


/*#include <dviglo/main/main.hpp>


i32 app_main(const vector<StrUtf8>& args)
{
    unique_ptr<App> app = make_unique<App>(args);
    return app->run();
}

DV_DEFINE_MAIN(app_main);
*/
