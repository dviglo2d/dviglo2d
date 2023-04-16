// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dviglo/main/application.hpp>
#include <dviglo/main/timer.hpp>


namespace dviglo
{

// Снижает FPS, если пользователь ничего не делает
class ImGuiApplication : public Application
{
protected:
    // Максмальный FPS
    i32 iga_high_fps_ = 200;

    // Сниженный FPS
    i32 iga_low_fps_ = 30;

    // Сколько времени пользователь ничего не делает
    i64 iga_idle_time_ns_ = 0;

    // Сколько времени пользователь должен бездействовать, чтобы FPS был снижен
    i64 iga_max_idle_time_ns_ = ms_to_ns(100);

    // Снижен ли FPS
    bool iga_is_low_fps() const { return iga_idle_time_ns_ >= iga_max_idle_time_ns_; }

    // Пользователь должен вызвать эту функцию в начале start()
    void iga_start();

    // Пользователь должен вызвать эту функцию в начале handle_sdl_event(...)
    void iga_handle_sdl_event(const SDL_Event& event);

    // Пользователь должен вызвать эту функцию в начале update(...)
    void iga_update(i64 ns);

    // Рендерит интерфейс ImGui
    void iga_draw();

public:
    // Наследуем конструкторы
    using Application::Application;

    ~ImGuiApplication() override;
};

} // namespace dviglo
