// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "os_window.hpp"

#include "../audio/audio.hpp"
#include "../fs/log.hpp"
#include "../gl_utils/shader_cache.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../res/freetype.hpp"
#include "../std_utils/scope_guard.hpp"

#include <SDL3/SDL.h>

#include <memory>


namespace dviglo
{

class Application
{
private:
    // Аргументы командной строки
    std::vector<StrUtf8> args_;

    // Порядок подсистем важен, так как влияет на очерёдность вызовов деструкторов
    std::unique_ptr<Log> log_;
    std::unique_ptr<OsWindow> os_window_;
    std::unique_ptr<ShaderCache> shader_cache_;
    std::unique_ptr<TextureCache> texture_cache_;
    std::unique_ptr<Audio> audio_;
    std::unique_ptr<FreeType> freetype_;

#ifdef DV_CTEST
    // Через сколько секунд после запуска приложение автоматически закроется.
    // При значении 0 закрываться не будет.
    // Задаётся с помощью параметра -duration x
    i64 duration_ = 0;
#endif

protected:
    // Пользователь желает прервать главный цикл
    bool should_exit_ = false;

    Application(const std::vector<StrUtf8>& args);
    virtual ~Application() = default;

    virtual void setup() {}
    virtual void start() {}
    virtual void new_frame() {}

    // Обработчики событий вызываются перед update()
    virtual void handle_sdl_event(const SDL_Event& event);

    virtual void update(i64 ns) { (void)ns; }
    virtual void draw() {}

public:
    const std::vector<StrUtf8>& args() const { return args_; }

    // Методы ниже должны быть публичными, чтобы SDL мог их вызвать.
    // Пользователь не должен их вызывать

    SDL_AppResult main_init();
    SDL_AppResult main_iterate();
    SDL_AppResult main_event(SDL_Event* event);
};

} // namespace dviglo
