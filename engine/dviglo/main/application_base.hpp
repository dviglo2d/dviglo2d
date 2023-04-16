// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "os_window.hpp"

#include "../audio/audio.hpp"
#include "../gl_utils/shader_cache.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../res/freetype.hpp"

#include <dv_log.hpp>
#include <SDL3/SDL.h>

#include <memory>


namespace dviglo
{

class ApplicationBase : public Subsystem
{
private:
    // Порядок подсистем важен, так как влияет на очерёдность вызовов деструкторов
    std::unique_ptr<OsWindow> os_window_;
    std::unique_ptr<ShaderCache> shader_cache_;
    std::unique_ptr<TextureCache> texture_cache_;
    std::unique_ptr<Audio> audio_;
    std::unique_ptr<FreeType> freetype_;

    // Вызван ли уже метод on_frame_begin()
    bool is_on_frame_begin_called = false;

#ifdef DV_CTEST
    // Через сколько секунд после запуска приложение автоматически закроется.
    // При значении 0 закрываться не будет.
    // Задаётся с помощью параметра -duration x
    i64 duration_ = 0;
#endif

protected:
    // Если <= 0, то без ограничений.
    // Переменную можно менять в любое время, от неё зависит задержка в конце кадра
    void max_fps(double value) const { SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, std::to_string(value).c_str()); }
    double max_fps() const { return SDL_atof(SDL_GetHint(SDL_HINT_MAIN_CALLBACK_RATE)); }

    // Пользователь желает прервать главный цикл
    bool should_exit_ = false;

    ApplicationBase();
    virtual ~ApplicationBase() = default;

    virtual void setup() {}
    virtual void start() {}

    // Метод вызывается перед обработчиками событий
    virtual void on_frame_begin() {}

    // Обработчики событий вызываются перед update()
    virtual void handle_sdl_event(const SDL_Event& event);

    virtual void update(i64 ns) { (void)ns; }
    virtual void draw() {}

    virtual void on_frame_end() {}

public:
    // Методы ниже должны быть публичными, чтобы SDL мог их вызвать.
    // Пользователь не должен их вызывать

    SDL_AppResult main_init();
    SDL_AppResult main_event(const SDL_Event* event);
    SDL_AppResult main_iterate();
};

} // namespace dviglo
