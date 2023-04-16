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
template <class Application>
struct MainSubsystems
{
    // Порядок полей важен, так как влияет на очерёдность вызовов деструкторов
    std::unique_ptr<MainArgs> main_args;
    std::unique_ptr<Log> log;
    std::unique_ptr<Application> application;

    SDL_AppResult init(i32 argc, char* argv[])
    {
        if (!set_locale())
            return SDL_APP_FAILURE;

        main_args = std::make_unique<MainArgs>(argc, argv);
        log = std::make_unique<Log>(Application::dv_get_log_path());

        if (!SDL_Init(0))
            return SDL_APP_FAILURE;

        application = std::make_unique<Application>();

        return application->main_init();
    }
};

} // namespace dviglo


SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    using Subsystems = dviglo::MainSubsystems<DV_APPLICATION_CLASS>;
    Subsystems* subsystems = new Subsystems();
    *appstate = subsystems;
    return subsystems->init(argc, argv);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    using Subsystems = dviglo::MainSubsystems<DV_APPLICATION_CLASS>;
    Subsystems* subsystems = static_cast<Subsystems*>(appstate);
    return subsystems->application->main_event(event);
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    using Subsystems = dviglo::MainSubsystems<DV_APPLICATION_CLASS>;
    Subsystems* subsystems = static_cast<Subsystems*>(appstate);
    return subsystems->application->main_iterate();
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    (void)result;
    using Subsystems = dviglo::MainSubsystems<DV_APPLICATION_CLASS>;
    Subsystems* subsystems = static_cast<Subsystems*>(appstate);
    delete subsystems;
}
