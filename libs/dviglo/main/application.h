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

    /// Аргументы командной строки
    const std::vector<StrUtf8>& args_;

protected:
    // Параметры движка, используемые при инициализации

    std::string log_path_;

    Application(const std::vector<StrUtf8>& args);
    ~Application();

    virtual void start() = 0;
    virtual void update(u64 ms) = 0;

public:
    SDL_Window* window() const { return window_; }

    i32 run();
};

} // namespace dviglo
