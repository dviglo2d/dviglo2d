// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "main_args.hpp"

#include <dv_log.hpp>
#include <memory>
#include <SDL3/SDL.h>


namespace dviglo
{

bool set_locale();

// Эту структуру SDL хранит внутри себя и передаёт в колбэки
struct MainSubsystems
{
    // Порядок полей важен, так как влияет на очерёдность вызовов деструкторов
    std::unique_ptr<MainArgs> main_args;
    std::unique_ptr<Log> log;
    std::unique_ptr<DV_CONFIG_CLASS> config;
    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<ShaderCacheOld> shader_cache_old;
    std::unique_ptr<TextureCache> texture_cache;
    std::unique_ptr<TextureSet> texture_set;
    std::unique_ptr<Audio> audio;
    std::unique_ptr<FreeType> freetype;
    std::unique_ptr<DV_APPLICATION_CLASS> application;

    SDL_AppResult init(i32 argc, char* argv[])
    {
        if (!set_locale())
            return SDL_APP_FAILURE;

        main_args = std::make_unique<MainArgs>(argc, argv);
        log = std::make_unique<Log>(DV_CONFIG_CLASS::log_path());

        if (!SDL_Init(0))
        {
            Log::writef_error("{} | !SDL_Init(0) | {}", DV_FUNC_SIG, SDL_GetError());
            return SDL_APP_FAILURE;
        }

        config = std::make_unique<DV_CONFIG_CLASS>();

        graphics = std::make_unique<Graphics>(*config);
        if (!graphics->is_valid())
        {
            Log::writef_error("{} | !Graphics->is_valid()", DV_FUNC_SIG);
            return SDL_APP_FAILURE;
        }

        shader_cache_old = std::make_unique<ShaderCacheOld>();
        texture_cache = std::make_unique<TextureCache>();

        texture_set = std::make_unique<TextureSet>(DV_GRAPHICS->vma_allocator(), DV_GRAPHICS->graphics_queue(), DV_GRAPHICS->graphics_queue_index());

        if (!texture_set->is_valid())
        {
            Log::writef_error("{} | !texture_set->is_valid()", DV_FUNC_SIG);
            return SDL_APP_FAILURE;
        }

        audio = std::make_unique<Audio>();
        freetype = std::make_unique<FreeType>();
        application = std::make_unique<DV_APPLICATION_CLASS>();

        return SDL_APP_CONTINUE;
    }
};

} // namespace dviglo


SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    dviglo::MainSubsystems* subsystems = new dviglo::MainSubsystems();
    *appstate = subsystems;
    return subsystems->init(argc, argv);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    dviglo::MainSubsystems* subsystems = static_cast<dviglo::MainSubsystems*>(appstate);
    return subsystems->application->main_event(event);
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    dviglo::MainSubsystems* subsystems = static_cast<dviglo::MainSubsystems*>(appstate);
    return subsystems->application->main_iterate();
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    (void)result;
    dviglo::MainSubsystems* subsystems = static_cast<dviglo::MainSubsystems*>(appstate);
    delete subsystems;
}
