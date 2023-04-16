#include "app.hpp"
#include "config.hpp"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define DV_CONFIG_CLASS Config
#define DV_APPLICATION_CLASS App
#include <dviglo/main/main.hpp>
