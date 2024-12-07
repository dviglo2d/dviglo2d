#include "app.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/main/engine_params.hpp>
#include <dviglo/main/os_window.hpp>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <misc/cpp/imgui_stdlib.h>

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
    engine_params::window_title = "Bitmap Font Generator";
}

void App::start()
{
    freetype_library_ = make_unique<FreeTypeLibrary>();

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

    static ImWchar full_range[] {0x0001, 0xFFFF, 0};
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 20.f, nullptr, full_range);

    ImGui::StyleColorsDark();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.1f, 0.15f, 1.f);

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

static f32 calc_button_width(const StrUtf8& label)
{
    f32 label_width = ImGui::CalcTextSize(label.c_str()).x;
    return label_width + ImGui::GetStyle().FramePadding.x * 2.f;
}

enum class FontStyle : i32
{
    simple = 0,
    outlined,   // С обводкой
    contour,    // Только контур
};

void App::show_ui()
{
    using namespace ImGui;

    static f32 f = 0.0f;
    static i32 counter = 0;
    static bool show_demo_window = true;
    static bool show_another_window = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static StrUtf8 src_font_path = get_base_path() + "font_generator_data/fonts/ubuntu/Ubuntu-R.ttf";
    static i32 font_height = 60; // В пикселях
    static i32 font_style_idx = (i32)FontStyle::simple;
    static ImVec4 main_color = ImVec4(1.f, 1.f, 1.f, 1.f);
    static ImVec4 second_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    static i32 thickness = 10;
    static i32 blur_radius = 8;
    static i32 texture_width_idx = 2;
    static i32 texture_height_idx = 2;

    ImGuiStyle& style = GetStyle();

    ShowDemoWindow(&show_demo_window);

    {
        SetNextWindowPos(ImVec2(20.f, 20.f), ImGuiCond_FirstUseEver);
        const f32 window_width = 500.f;
        SetNextWindowSizeConstraints(ImVec2(window_width, 0.f), ImVec2(window_width, FLT_MAX));
        Begin("Настройки шрифта", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        PushItemWidth(-FLT_MIN); // Label элементов не используем

        // Исходный шрифт
        { 
            Text("Исходный шрифт");

            const StrUtf8 explore_button_label = "...";
            PushItemWidth(-calc_button_width(explore_button_label) - style.ItemSpacing.x);
            InputText("##src_font_path", &src_font_path, ImGuiInputTextFlags_ElideLeft);
            PopItemWidth();

            SameLine();
            Button(explore_button_label.c_str());
        }

        NewLine();

        // Высота
        {
            Text("Высота");

            SameLine();
            SliderInt("##font_height", &font_height, 4, 100);
            SetItemTooltip("В пикселях\n\nРеальная высота символов в текстуре может\nувеличиваться за счёт обводки и размытия");
        }

        NewLine();

        // Стиль
        {
            Text("Стиль");

            SameLine();
            const char* items[] = {"Простой", "С обводкой", "Только контур"};
            Combo("##style_idx", &font_style_idx, items, IM_ARRAYSIZE(items));
        }

        // Простой стиль
        if (font_style_idx == (i32)FontStyle::simple)
        {
            Bullet();
            Text("Цвет");
            SameLine();
            ColorEdit4("##main_color", (f32*)&main_color, ImGuiColorEditFlags_AlphaPreview);

            Bullet();
            Text("Радиус размытия");
            SameLine();
            SliderInt("##blur_radius", &blur_radius, 0, 20);
        }
        // С обводкой
        else if (font_style_idx == (i32)FontStyle::outlined)
        {
            Bullet();
            Text("Основной цвет");
            SameLine();
            ColorEdit4("##main_color", (f32*)&main_color, ImGuiColorEditFlags_AlphaPreview);

            Bullet();
            Text("Обводка");
            Indent(GetFontSize());
                Bullet();
                Text("Цвет");
                SameLine();
                ColorEdit4("##second_color", (f32*)&second_color, ImGuiColorEditFlags_AlphaPreview);

                Bullet();
                Text("Толщина");
                SameLine();
                SliderInt("##thickness", &thickness, 0, 20);

                Bullet();
                Text("Радиус размытия");
                SameLine();
                SliderInt("##blur_radius", &blur_radius, 0, 20);
            Unindent(GetFontSize());
        }
        // Только контур
        else if (font_style_idx == (i32)FontStyle::contour)
        {
            Bullet();
            Text("Цвет");
            SameLine();
            ColorEdit4("##second_color", (f32*)&second_color, ImGuiColorEditFlags_AlphaPreview);

            Bullet();
            Text("Толщина");
            SameLine();
            SliderInt("##thickness", &thickness, 1, 20);

            Bullet();
            Text("Радиус размытия");
            SameLine();
            SliderInt("##blur_radius", &blur_radius, 0, 20);
        }

        NewLine();

        // Размер текстуры
        {
            Text("Размер текстуры");

            const char* items[] = {"256", "512", "1024", "2048"};

            SameLine();
            f32 combo_width = (GetContentRegionAvail().x - style.ItemSpacing.x) * 0.5f;
            PushItemWidth(-combo_width - style.ItemSpacing.x);
            Combo("##texture_width_idx", &texture_width_idx, items, IM_ARRAYSIZE(items));
            SetItemTooltip("Ширина");
            PopItemWidth();

            SameLine();
            Combo("##texture_height_idx", &texture_height_idx, items, IM_ARRAYSIZE(items));
            SetItemTooltip("Высота");
        }

        PopItemWidth(); // Label элементов не используем

        NewLine();

        // Кнопка генерации
        {
            // Центрируем кнопку
            const StrUtf8 generate_button_label = "Генерировать!";
            f32 size = calc_button_width(generate_button_label);
            f32 avail = GetContentRegionAvail().x;
            f32 offset = (avail - size) * 0.5f;
            SetCursorPosX(GetCursorPosX() + offset);

            if (Button(generate_button_label.c_str()))
            {
                unique_ptr<FreeTypeFace> face = make_unique<FreeTypeFace>(*freetype_library_, src_font_path, font_height);
            }
        }

        NewLine();

        ImGui::End();
    }
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
