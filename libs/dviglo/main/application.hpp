// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../io/log.hpp"

#include <entt/entt.hpp>
#include <SDL3/SDL.h>


namespace dviglo
{

class Application
{
private:
    /// Аргументы командной строки
    const std::vector<StrUtf8>& args_;

protected:
    // Параметры движка, используемые при инициализации

    StrUtf8 log_path_;

#ifdef DV_CTEST
    /// Через сколько секунд после запуска приложение автоматически закроется.
    /// При значении 0 закрываться не будет.
    /// Задаётся с помощью параметра -duration x
    u64 duration_ = 0;
#endif

    /// Пользователь желает прервать главный цикл
    bool should_exit_ = false;

    Application(const std::vector<StrUtf8>& args);
    ~Application();

    virtual void setup() = 0;
    virtual void start() = 0;
    virtual void update(u64 ns) = 0;
    virtual void draw() = 0;

    virtual void on_key(const SDL_KeyboardEvent& event_data);

public:
    const std::vector<StrUtf8>& args() const { return args_; }

    i32 run();
};

} // namespace dviglo
