// Copyright (c) the Dviglo project
// License: MIT

#include "dv_imgui_application.hpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>


namespace dviglo
{

ImGuiApplication::~ImGuiApplication()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiApplication::iga_start()
{
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForOpenGL(DV_OS_WINDOW->window(), nullptr);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void ImGuiApplication::iga_handle_sdl_event(const SDL_Event& event)
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

void ImGuiApplication::iga_update(i64 ns)
{
    if (iga_idle_time_ns_ < iga_max_idle_time_ns_)
        iga_idle_time_ns_ += ns;

    // Не снижаем FPS, если пользователь зажимает любую кнопку мыши
    SDL_MouseButtonFlags mouse_state = SDL_GetMouseState(nullptr, nullptr);
    if (mouse_state)
        iga_idle_time_ns_ = 0;

    if (iga_is_low_fps())
        max_fps_ = iga_low_fps_;
    else
        max_fps_ = iga_high_fps_;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiApplication::iga_draw()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace dviglo
