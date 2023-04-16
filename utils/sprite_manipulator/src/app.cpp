#include "app.hpp"

#include <imgui.h>

#include <misc/freetype/imgui_freetype.h>

using namespace glm;


App::App()
{
    fs::path pref_path = get_pref_path("dviglo2d", "sprite_manipulator");
    fs::path base_path = get_base_path();
    fs::path font_path = base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf";

    ImGuiIO& io = ImGui::GetIO();
    static StrUtf8 ini_path = (pref_path / "imgui.ini").string();
    io.IniFilename = ini_path.c_str();
    static StrUtf8 log_path = (pref_path / "imgui.log").string();
    io.LogFilename = log_path.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    static ImWchar full_range[] {0x0001, 0xFFFF, 0};
    static ImFontConfig font_config;
    font_config.FontLoaderFlags |= ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), 20.f, &font_config, full_range);

    ImGui::StyleColorsDark();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.1f, 0.15f, 1.f);
}

App::~App()
{
}

void App::handle_sdl_event(const SDL_Event& event)
{
    iga_handle_sdl_event(event);

    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        on_key(event.key);
        return;

    default:
        // Реагируем на закрытие приложения и изменение размера окна
        ImGuiApplication::handle_sdl_event(event);
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
    (void)ns;

    //ImGui::ShowDemoWindow(nullptr);

    // Главное меню
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Файл"))
            {
                if (ImGui::MenuItem("Открыть"))
                {
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    // Status bar
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    f32 status_bar_height = ImGui::GetTextLineHeight() + 2 * ImGui::GetStyle().WindowPadding.y;
    {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - status_bar_height));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, status_bar_height));
        ImGui::Begin("##status_bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        StrUtf8 str = format("FPS: {}", fps());
        if (iga_is_low_fps())
            str += " (снижен)";
        ImGui::TextUnformatted(str.c_str());

        ImGui::End();
    }

    // Окно на всю свободную область
    {
        // WorkPos и WorkSize уже учитывают наличие главного меню
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImVec2 workspace_size(viewport->WorkSize.x, viewport->WorkSize.y - status_bar_height);
        ImGui::SetNextWindowSize(workspace_size);
        ImGui::Begin("##main_window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
        ImGui::TextUnformatted("TODO");
        ImGui::End();
    }
}

void App::update(i64 ns)
{
    iga_update(ns);
    show_ui(ns);
}

void App::draw()
{
    glClearColor(0x3a/255.f, 0x6f/255.f, 0xa5/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    iga_draw();
}
