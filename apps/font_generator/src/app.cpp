#include "app.hpp"

#include "result_window.hpp"
#include "settings_window.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/main/engine_params.hpp>
#include <dviglo/main/timer.hpp>
#include <dviglo/math/math.hpp>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <misc/cpp/imgui_stdlib.h>
#include <misc/freetype/imgui_freetype.h>

using namespace glm;


App::App(const vector<StrUtf8>& args)
    : Application(args)
{
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

static StrUtf8 pref_path = get_pref_path("dviglo2d", "font_generator");

void App::setup()
{
    engine_params::log_path = pref_path + "app.log";
    engine_params::window_size = {1100, 600};
    engine_params::window_mode = WindowMode::resizable;
    engine_params::window_title = "Bitmap Font Generator";
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    StrUtf8 font_path = base_path + "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    static StrUtf8 ini_path = pref_path + "imgui.ini";
    io.IniFilename = ini_path.c_str();
    static StrUtf8 log_path = pref_path + "imgui.log";
    io.LogFilename = log_path.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    static ImWchar full_range[] {0x0001, 0xFFFF, 0};
    static ImFontConfig font_config;
    font_config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 20.f, &font_config, full_range);

    ImGui::StyleColorsDark();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.1f, 0.15f, 1.f);

    ImGui_ImplSDL3_InitForOpenGL(DV_OS_WINDOW->window(), nullptr);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    preview_window_ = make_unique<PreviewWindow>();
}

// Когда to_low_fps_timer достигнет нуля, FPS будет снижен.
// Для плавности FPS максимальнен, когда пользователь
// перетаскивает окно приложения или окно ImGui
constexpr i64 high_fps_time = ms_to_ns(100);
static i64 to_low_fps_timer = high_fps_time;

void App::handle_sdl_event(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);

    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        on_key(event.key);
        return;

    case SDL_EVENT_WINDOW_MOVED:
    case SDL_EVENT_WINDOW_RESIZED:
        // Не снижаем FPS
        to_low_fps_timer = high_fps_time;
        [[fallthrough]];

    default:
        // Реагируем на закрытие приложения и изменение размера окна
        Application::handle_sdl_event(event);
        return;
    }
}

void App::on_key(const SDL_KeyboardEvent& event_data)
{
    if (event_data.type == SDL_EVENT_KEY_DOWN && event_data.repeat == false
        && event_data.scancode == SDL_SCANCODE_ESCAPE)
    {
        should_exit_ = true;
    }
}

void App::show_ui(i64 ns)
{
    using namespace ImGui;

    // Время, которое пользователь не меняет никакие настройки шрифта.
    // Если меньше нуля, то шрифт уже сгенерирован
    static i64 idle_time_ns = 0;

    if (idle_time_ns >= 0)
        idle_time_ns += ns;

    // Сколько времени генерировался шрифт
    static i64 generation_time_ms = 0;

    static FileDialogState file_dialog_state = FileDialogState::closed;

    //ShowDemoWindow(nullptr);

    // Используем строку меню для отображения ФПС
    {
        if (ImGui::BeginMainMenuBar())
        {
            ImGuiIO& io = ImGui::GetIO();
            StrUtf8 str = format("Время кадра: {} мс | FPS: {}", i32(1000 / io.Framerate), (i32)io.Framerate);

            if (to_low_fps_timer <= 0)
                str += " | Включено ограничение FPS";

            ImGui::TextUnformatted(str.c_str());

            ImGui::EndMainMenuBar();
        }
    }

    show_settings_window(generated_font_, generation_time_ms, idle_time_ns, file_dialog_state);
    show_result_window(generated_font_.get(), generation_time_ms, file_dialog_state);
    preview_window_->show(generated_font_.get());
    show_open_dialog(idle_time_ns, file_dialog_state);
    show_save_dialog(generated_font_.get(), file_dialog_state);
}

void App::update(i64 ns)
{
    if (to_low_fps_timer > 0)
        to_low_fps_timer -= ns;

    // Не снижаем FPS, если пользователь зажимает любую кнопку мыши
    Uint32 mouse_state = SDL_GetMouseState(nullptr, nullptr);
    if (mouse_state)
        to_low_fps_timer = high_fps_time;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    show_ui(ns);
}

void App::draw()
{
    glClearColor(0x3a/255.f, 0x6f/255.f, 0xa5/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Снижаем FPS
    if (to_low_fps_timer <= 0)
        delay_ms(25);
}
