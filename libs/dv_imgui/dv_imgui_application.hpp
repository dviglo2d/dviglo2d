// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_math.hpp>
#include <dv_sdl_utils.hpp>
#include <dviglo/main/application_base.hpp>
#include <dviglo/main/timer.hpp>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>


namespace dviglo
{

// Снижает FPS, если пользователь ничего не делает
class ImGuiApplication : public ApplicationBase
{
private:
    // Текущий FPS
    i32 fps_ = 0;

    // Обновляет переменную fps_
    void update_fps_value(i64 ns)
    {
        static i64 frame_counter = 0;
        static i64 time_counter = 0;

        ++frame_counter;
        time_counter += ns;

        // Обновляем fps_ каждые пол секунды
        if (time_counter >= ns_per_s / 2)
        {
            fps_ = round_to_i32(static_cast<f64>(frame_counter * ns_per_s) / static_cast<f64>(time_counter));
            frame_counter = 0;
            time_counter = 0;
        }
    }

protected:
    // Максмальный FPS
    f64 iga_high_fps_ = 200.0;

    // Сниженный FPS
    f64 iga_low_fps_ = 30.0;

    // Сколько времени пользователь ничего не делает
    i64 iga_idle_time_ns_ = 0;

    // Сколько времени пользователь должен бездействовать, чтобы FPS был снижен
    i64 iga_max_idle_time_ns_ = ms_to_ns(100);

    // Снижен ли FPS
    bool iga_is_low_fps() const { return iga_idle_time_ns_ >= iga_max_idle_time_ns_; }

    // Текущий FPS. Обновляется каждые пол секунды, в отличие от io.Framerate, который обновляется каждый кадр
    i32 fps() const { return fps_; }

    ImGuiApplication()
    {
        ImGui::CreateContext();
        ImGui_ImplSDL3_InitForOpenGL(DV_OS_WINDOW->window(), nullptr);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    }

    // Пользователь должен вызвать эту функцию в начале handle_sdl_event(...)
    void iga_handle_sdl_event(const SDL_Event& event)
    {
        switch (event.type)
        {
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_RESIZED:
            iga_idle_time_ns_ = 0;
            break;
        }

        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    // Пользователь должен вызвать эту функцию в начале update(...)
    void iga_update(i64 ns)
    {
        if (iga_idle_time_ns_ < iga_max_idle_time_ns_)
            iga_idle_time_ns_ += ns;

        // Не снижаем FPS, если пользователь зажимает любую кнопку мыши
        SDL_MouseButtonFlags mouse_state = SDL_GetMouseState(nullptr, nullptr);
        if (mouse_state)
            iga_idle_time_ns_ = 0;

        if (iga_is_low_fps()) // iga_idle_time_ns_ >= iga_max_idle_time_ns_
            max_fps(iga_low_fps_);
        else
            max_fps(iga_high_fps_);

        // Обновляем переменную fps_
        update_fps_value(ns);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    // Рендерит интерфейс ImGui
    void iga_draw()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

public:
    ~ImGuiApplication() override
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
};

} // namespace dviglo
