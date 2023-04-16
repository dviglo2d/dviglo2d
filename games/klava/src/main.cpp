#include "app.hpp"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define DV_APPLICATION_CLASS App
#include <dviglo/main/main.hpp>

using namespace dviglo;


fs::path dv_get_log_path()
{
    return get_pref_path("dviglo2d", "games") / "klava.log";
}
