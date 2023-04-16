// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "os_window.hpp"

#include "engine_params.hpp"
#include "main_args.hpp"
#include "timer.hpp"

#include "../audio/audio.hpp"
#include "../gl_utils/shader_cache.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../res/freetype.hpp"

#include <dv_log.hpp>
#include <glad/gl.h>
#include <SDL3/SDL.h>

#include <memory>


namespace dviglo
{

class ApplicationBase : public SubsystemIndex
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

    ApplicationBase()
    {
        const std::vector<StrUtf8>& args = DV_MAIN_ARGS->get();

        // Нулевой аргумент - запускаемый файл, поэтому пропускаем
        for (size_t i = 1; i < args.size(); ++i)
        {
            const StrUtf8& arg = args[i];

            if (arg == "-duration")
            {
                // Получаем следующий аргумент, если есть
                if (i + 1 < args.size())
                {
                    ++i;
#ifdef DV_CTEST
                    duration_ = to_u64(args[i]);
#endif
                }
            }
        }
    }

    virtual void setup() {}
    virtual void start() {}

    // Метод вызывается перед обработчиками событий
    virtual void on_frame_begin() {}

    // Обработчики событий вызываются перед update()
    virtual void handle_sdl_event(const SDL_Event& event)
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            should_exit_ = true;
            return;

        case SDL_EVENT_WINDOW_RESIZED:
            {
                i32 width = event.window.data1;
                i32 height = event.window.data2;
                glViewport(0, 0, width, height);
                return;
            }
        }
    }

    virtual void update(i64 ns) { (void)ns; }
    virtual void draw() {}
    virtual void on_frame_end() {}

public:
    // Пользователь может переопределить этот метод.
    // Метод статический, так как сперва открывается файл лога, а только потом создаётся текущий объект
    static fs::path dv_get_log_path() { return get_pref_path("dviglo2d", "default") / "default.log"; }

    void exit() { should_exit_ = true; }

    // Методы ниже должны быть публичными, чтобы SDL мог их вызвать.
    // Пользователь не должен их вызывать

    SDL_AppResult main_init()
    {
        setup();

        os_window_ = std::make_unique<OsWindow>();
        shader_cache_ = std::make_unique<ShaderCache>();
        texture_cache_ = std::make_unique<TextureCache>();
        audio_ = std::make_unique<Audio>();
        freetype_ = std::make_unique<FreeType>();

        start();

        return SDL_APP_CONTINUE;
    }

    SDL_AppResult main_event(const SDL_Event* event)
    {
        if (!is_on_frame_begin_called)
        {
            on_frame_begin();
            is_on_frame_begin_called = true;
        }

        handle_sdl_event(*event);

        if (should_exit_)
            return SDL_APP_SUCCESS;
        else
            return SDL_APP_CONTINUE;
    }

    SDL_AppResult main_iterate()
    {
#ifdef DV_CTEST
        // При CTest выходим через duration_ секунд после запуска приложения
        if (duration_ && get_ticks_ns() >= s_to_ns(duration_))
            should_exit_ = true;
#endif

        static i64 old_ticks_ns = get_ticks_ns(); // Время начала прошлого кадра
        i64 new_ticks_ns = get_ticks_ns(); // Время начала кадра
        i64 ns = new_ticks_ns - old_ticks_ns;
        assert(ns >= 0);
        old_ticks_ns = new_ticks_ns;

        // Если точности get_ticks_ns() не хватает
        if (ns == 0)
        {
            // Ждём полмиллисекунды
            delay_ns(ns_per_ms / 2);
            return SDL_APP_CONTINUE;
        }

        if (!is_on_frame_begin_called)
        {
            on_frame_begin();
            is_on_frame_begin_called = true;
        }

        update(ns);
        draw();
        SDL_GL_SwapWindow(DV_OS_WINDOW->window());

        // Убираем буферизацию и инпут лаг при включённой вертикалке на NVIDIA в Windows.
        // По ощущениям работает лучше, когда стоит после SDL_GL_SwapWindow(...), а не до
        glFinish();

        on_frame_end();
        is_on_frame_begin_called = false;

        if (should_exit_)
            return SDL_APP_SUCCESS;
        else
            return SDL_APP_CONTINUE;
    }
};

} // namespace dviglo
