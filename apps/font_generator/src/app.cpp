#include "app.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/main/engine_params.hpp>
#include <dviglo/main/os_window.hpp>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include <format>

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
    engine_params::window_size = {900, 700};
    engine_params::msaa_samples = 4; // При значении 8 крэшится на сервере ГитХаба в Линуксе
    engine_params::window_mode = WindowMode::resizable;
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    StrUtf8 font_path = base_path + "font_generator_data/fonts/ubuntu/Ubuntu-R.ttf";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    static StrUtf8 ini_path = pref_path + "imgui.ini";
    io.IniFilename = ini_path.c_str();
    static StrUtf8 log_path = pref_path + "imgui.log";
    io.LogFilename = log_path.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 20.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(DV_OS_WINDOW->window(), nullptr);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

// Когда to_low_fps_timer достигнет нуля, FPS будет снижен.
// Для плавности FPS максимальнен, когда пользователь
// перетаскивает окно приложения или окно ImGui
constexpr u64 high_fps_time = SDL_NS_PER_MS * 100;
static u64 to_low_fps_timer = high_fps_time;

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

void App::show_ui()
{
    static float f = 0.0f;
    static int counter = 0;
    static bool show_demo_window = true;
    static bool show_another_window = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Кнопка"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}

void App::update(u64 ns)
{
    if (to_low_fps_timer >= ns)
        to_low_fps_timer -= ns;
    else
        to_low_fps_timer = 0;

    // Не снижаем FPS, если пользователь перетаскивает окно ImGui
    Uint32 mouse_state = SDL_GetMouseState(nullptr, nullptr);
    if (mouse_state)
        to_low_fps_timer = high_fps_time;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    show_ui();
}

void App::draw()
{
    glClearColor(0x3a/255.f, 0x6f/255.f, 0xa5/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (to_low_fps_timer == 0)
        SDL_Delay(25);
}
