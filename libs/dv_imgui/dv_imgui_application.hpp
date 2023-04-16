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
private:
    // Текущий FPS
    i32 fps_ = 0;

    // Обновляет переменную fps_
    void update_fps_value(i64 ns);

protected:
    // Максмальный FPS
    double iga_high_fps_ = 200.0;

    // Сниженный FPS
    double iga_low_fps_ = 30.0;

    // Сколько времени пользователь ничего не делает
    i64 iga_idle_time_ns_ = 0;

    // Сколько времени пользователь должен бездействовать, чтобы FPS был снижен
    i64 iga_max_idle_time_ns_ = ms_to_ns(100);

    // Снижен ли FPS
    bool iga_is_low_fps() const { return iga_idle_time_ns_ >= iga_max_idle_time_ns_; }

    // Текущий FPS. Обновляется каждые пол секунды, в отличие от io.Framerate, который обновляется каждый кадр
    i32 fps() const { return fps_; }

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
