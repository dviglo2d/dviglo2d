#include "app.hpp"

#include <dviglo/main/engine_params.hpp>
#include <dviglo/main/os_window.hpp>
#include <dviglo/math/math.hpp>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <misc/freetype/imgui_freetype.h>

#include <format>


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
    static ImFontConfig font_config;
    font_config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 20.f, &font_config, full_range);

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

void App::show_ui()
{
    using namespace ImGui;

    static f32 f = 0.0f;
    static i32 counter = 0;
    static bool show_demo_window = true;
    static bool show_another_window = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static dviglo::Image glyph_image;
    static Texture glyph_texture;

    static FontSettings font_settings;

    ImGuiStyle& style = GetStyle();

    ShowDemoWindow(&show_demo_window);

    // Окно настройки шрифта
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
            InputText("##src_font_path", &font_settings.src_path, ImGuiInputTextFlags_ElideLeft);
            PopItemWidth();

            SameLine();
            Button(explore_button_label.c_str());
        }

        NewLine();

        // Высота
        {
            Text("Высота");

            SameLine();
            SliderInt("##font_height", &font_settings.height, 4, 120);
            SetItemTooltip("В пикселях\n\nРеальная высота символов в текстуре может\nувеличиваться за счёт обводки и размытия");
        }

        NewLine();

        // Стиль
        {
            Text("Стиль");

            SameLine();
            const char* items[] = {"Простой", "С обводкой", "Только контур"};
            static_assert(IM_ARRAYSIZE(items) == (i32)FontStyle::last + 1);
            Combo("##style_idx", (i32*)&font_settings.font_style, items, IM_ARRAYSIZE(items));
        }

        // Простой стиль
        if (font_settings.font_style == FontStyle::simple)
        {
            Bullet();
            Text("Цвет");
            SameLine();
            ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview);

            Bullet();
            Text("Радиус размытия");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val);
            }
        }
        // С обводкой
        else if (font_settings.font_style == FontStyle::outlined)
        {
            Bullet();
            Text("Основной цвет");
            SameLine();
            ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview);

            Bullet();
            Text("Обводка");

            Indent(GetFontSize());

            Bullet();
            Text("Цвет");
            SameLine();
            ColorEdit4("##second_color", (f32*)&font_settings.second_color, ImGuiColorEditFlags_AlphaPreview);

            Bullet();
            Text("Толщина");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
                SliderInt("##thickness", &font_settings.thickness, min_val, max_val);
            }

            Bullet();
            Text("Радиус размытия");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val);
            }

            Unindent(GetFontSize());
        }
        // Только контур
        else if (font_settings.font_style == FontStyle::contour)
        {
            Bullet();
            Text("Цвет");
            SameLine();
            ColorEdit4("##second_color", (f32*)&font_settings.second_color, ImGuiColorEditFlags_AlphaPreview);

            Bullet();
            Text("Толщина");
            SameLine();
            {
                constexpr i32 min_val = 1;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
                SliderInt("##thickness", &font_settings.thickness, min_val, max_val);
            }

            Bullet();
            Text("Радиус размытия");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val);
            }
        }

        NewLine();

        // Размер текстуры
        {
            Text("Размер текстуры");

            const vector<i32> sizes {256, 512, 1024, 2048};

            SameLine();
            f32 combo_width = (GetContentRegionAvail().x - style.ItemSpacing.x) * 0.5f;
            PushItemWidth(-combo_width - style.ItemSpacing.x);
            if (BeginCombo("##texture_width", to_string(font_settings.texture_size.x).c_str()))
            {
                for (i32 size : sizes)
                {
                    const bool is_selected = (size == font_settings.texture_size.x);
                    if (Selectable(to_string(size).c_str(), is_selected))
                        font_settings.texture_size.x = size;

                    if (is_selected)
                        SetItemDefaultFocus();
                }
                EndCombo();
            }
            SetItemTooltip("Ширина");
            PopItemWidth();

            SameLine();
            if (BeginCombo("##texture_height", to_string(font_settings.texture_size.y).c_str()))
            {
                for (i32 size : sizes)
                {
                    const bool is_selected = (size == font_settings.texture_size.y);
                    if (Selectable(to_string(size).c_str(), is_selected))
                        font_settings.texture_size.y = size;

                    if (is_selected)
                        SetItemDefaultFocus();
                }
                EndCombo();
            }
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
                unique_ptr<FreeTypeFace> face = make_unique<FreeTypeFace>(font_settings);
                GeneratedFont generated_font = generate_font_simple(*face, font_settings);
                glyph_texture = Texture(generated_font.pages[0]);
            }
        }

        NewLine();

        End();
    }

    // Окно вывода текстур
    {
        SetNextWindowPos(ImVec2(20.f, 20.f), ImGuiCond_FirstUseEver);
        SetNextWindowSizeConstraints(ImVec2(10.f, 10.f), ImVec2(FLT_MAX, FLT_MAX));
        Begin("Сгенерированный шрифт");

        ImVec2 image_size((f32)glyph_texture.width(), (f32)glyph_texture.height());
        ImGui::Image(glyph_texture.gpu_object_name(), image_size);

        End();
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

    // Снижаем FPS
    if (to_low_fps_timer == 0)
        SDL_Delay(25);
}
