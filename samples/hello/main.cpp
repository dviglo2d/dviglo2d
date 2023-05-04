// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <dviglo/main/application.h>
#include <dviglo/main/main.h>
#include <dviglo/std_utils/scope_guard.h>

#include <entt/entt.hpp>
#include <SDL.h>

#include <memory>

using namespace dviglo;
using namespace std;


struct position {
    float x;
    float y;
};

struct velocity {
    float dx;
    float dy;
};

void update(entt::registry& registry) {
    auto view = registry.view<const position, velocity>();

    // use a callback
    view.each([](const auto& pos, auto& vel) { /* ... */ });

    // use an extended callback
    view.each([](const auto entity, const auto& pos, auto& vel) { /* ... */ });

    // use a range-for
    for (auto [entity, pos, vel] : view.each()) {
        // ...
    }

    // use forward iterators and get only the components of interest
    for (auto entity : view) {
        auto& vel = view.get<velocity>(entity);
        // ...
    }
}


class App : public Application
{
public:
    App()
    {
        log_path_ = "путь/к/логу";
    }
};


i32 run()
{
    unique_ptr<App> app = make_unique<App>();
    app->run();

    // Вызываем SDL_Quit(), даже если SDL_Init() вернул ошибку
    const ScopeGuard sg_sdl_quit = [] { SDL_Quit(); };

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    SDL_Window* window = SDL_CreateWindowWithPosition(
        "Окно закроется через 3 секунды",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600,
        0
    );

    if (!window)
        return 1;

    const ScopeGuard sg_destroy_window = [window] { SDL_DestroyWindow(window); };

    entt::registry registry;

    for (auto i = 0u; i < 10u; ++i) {
        const auto entity = registry.create();
        registry.emplace<position>(entity, i * 1.f, i * 1.f);
        if (i % 2 == 0) { registry.emplace<velocity>(entity, i * .1f, i * .1f); }
    }

    update(registry);

    SDL_Delay(3000);

    return 0;
}

DV_DEFINE_MAIN(run);
