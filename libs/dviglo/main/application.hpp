// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../fs/log.hpp"

#include <SDL3/SDL.h>


namespace dviglo
{

class Application
{
private:
    /// Аргументы командной строки
    std::vector<StrUtf8> args_;

#ifdef DV_CTEST
    /// Через сколько секунд после запуска приложение автоматически закроется.
    /// При значении 0 закрываться не будет.
    /// Задаётся с помощью параметра -duration x
    u64 duration_ = 0;
#endif

protected:
    /// Пользователь желает прервать главный цикл
    bool should_exit_ = false;

    Application(const std::vector<StrUtf8>& args);
    virtual ~Application() = default;

    virtual void setup() = 0;
    virtual void start() = 0;

    /// Обработчик событий вызывается перед update()
    virtual void handle_sdl_event(const SDL_Event& event);

    virtual void update(u64 ns) = 0;
    virtual void draw() = 0;

public:
    const std::vector<StrUtf8>& args() const { return args_; }

    // Методы ниже должны быть публичными, чтобы SDL мог их вызвать.
    // Пользователь не должен их вызывать

    SDL_AppResult main_init();
    SDL_AppResult main_iterate();
    SDL_AppResult main_event(SDL_Event* event);
    void main_quit(SDL_AppResult result);
};

} // namespace dviglo
