#include "app.hpp"

#include "config.hpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <misc/freetype/imgui_freetype.h>


App::App()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    fs::path pref_path = Config::pref_path();
    fs::path base_path = get_base_path();
    fs::path font_path = base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf";

    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForOpenGL(DV_OS_WINDOW->window(), nullptr);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    ImGuiIO& io = ImGui::GetIO();
    static StrUtf8 ini_path = (pref_path / "imgui.ini").string();
    io.IniFilename = ini_path.c_str();
    static StrUtf8 log_path = (pref_path / "imgui.log").string();
    io.LogFilename = log_path.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    static ImWchar full_range[]{ 0x0001, 0xFFFF, 0 };
    static ImFontConfig font_config;
    font_config.FontLoaderFlags |= ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 20.f, &font_config, full_range);

    ImGui::StyleColorsDark();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.1f, 0.15f, 1.f);

    app_state_manager_ = make_unique<AppStateManager>();
    app_state_manager_->required_app_state_id(AppStateId::main_screen);
}

App::~App()
{
    instance_ = nullptr;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void App::on_frame_begin()
{
    app_state_manager_->apply();
}

void App::handle_sdl_event(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);

    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        app_state_manager_->current_app_state()->on_key(event.key);
        return;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        app_state_manager_->current_app_state()->on_mouse_button(event.button);
        return;

    default:
        // Реагируем на закрытие приложения и изменение размера окна
        ApplicationBase::handle_sdl_event(event);
        return;
    }
}

void App::on_key(const SDL_KeyboardEvent& event_data)
{
    app_state_manager_->current_app_state()->on_key(event_data);
}

void App::on_mouse_button(const SDL_MouseButtonEvent& event_data)
{
    app_state_manager_->current_app_state()->on_mouse_button(event_data);
}

void App::update(i64 ns)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    app_state_manager_->current_app_state()->update(ns);
}

void App::draw()
{
    glClearColor(0x3a / 255.f, 0x6f / 255.f, 0xa5 / 255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    app_state_manager_->current_app_state()->draw();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
