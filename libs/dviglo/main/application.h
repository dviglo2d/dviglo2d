// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../io/log.h"

#include <entt/entt.hpp>


struct SDL_Window;

namespace dviglo
{

class DV_API Application
{
private:
    SDL_Window* window_ = nullptr;

    /// Главный объект ECS
    entt::registry ecs_;

protected:
    // Параметры движка, используемые при инициализации

    std::string log_path_;

    Application();
    ~Application();

public:
    SDL_Window* window() const { return window_; }

    /// Главный объект ECS
    entt::registry& ecs() { return ecs_; }

    i32 run();
};

} // namespace dviglo
