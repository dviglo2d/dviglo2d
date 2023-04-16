#include "app.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/main/engine_params.hpp>
#include <dviglo/math/math.hpp>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <misc/freetype/imgui_freetype.h>

#include <chrono>
#include <inttypes.h>

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
    engine_params::msaa_samples = 4; // При значении 8 крэшится на сервере ГитХаба в Линуксе
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

enum class FontStyle : u32
{
    simple = 0,
    outlined,         // С обводкой
    contour,          // Только контур
    last = contour    // Для определения числа стилей
};

struct FontSettings
{
    StrUtf8 src_path = get_base_path() + "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf";
    i32 height = 20; // В пикселях
    bool anti_aliasing = true;
    FontStyle font_style = FontStyle::simple;
    ImVec4 main_color{1.f, 1.f, 1.f, 1.f};   // Основной цвет
    ImVec4 second_color{0.f, 0.f, 0.f, 1.f}; // Дополнительный цвет
    i32 thickness = 10; // Толщина контура или обводки
    i32 blur_radius = 0;  // Радиус размытия
    ivec2 texture_size{1024, 1024};
};

enum class FileDialogState
{
    closed,
    open_file,
    save_file
};

void App::show_ui()
{
    using namespace ImGui;

    static i32 current_page = 0;
    static i64 generation_time = 0; // В миллисекундах

    static FontSettings font_settings;
    static FileDialogState file_dialog_state = FileDialogState::closed;

    static bool need_generate = true;
    auto now = chrono::steady_clock::now();
    static chrono::steady_clock::time_point last_interaction_time = now;

    ImGuiStyle& style = GetStyle();
    static StrUtf8 src_font_path_inside_dialog;

    //ShowDemoWindow(nullptr);

    // Окно настройки шрифта
    {
        SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_FirstUseEver);
        const f32 window_width = 500.f;
        SetNextWindowSizeConstraints(ImVec2(window_width, 0.f), ImVec2(window_width, FLT_MAX));
        Begin("Настройки шрифта", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        PushItemWidth(-FLT_MIN); // Label элементов не используем

        // Исходный шрифт
        { 
            TextUnformatted("Исходный шрифт");

            const StrUtf8 explore_button_label = "...";
            PushStyleVar(ImGuiStyleVar_Alpha, style.Alpha * style.DisabledAlpha); // Делаем текст серым
            PushItemWidth(-calc_button_width(explore_button_label) - style.ItemSpacing.x);
            InputText("##src_font_path", &font_settings.src_path, ImGuiInputTextFlags_ElideLeft | ImGuiInputTextFlags_ReadOnly);
            PopItemWidth();
            ImGui::PopStyleVar();

            SameLine();
            if (Button(explore_button_label.c_str()))
            {
                file_dialog_state = FileDialogState::open_file;
                src_font_path_inside_dialog = font_settings.src_path;
            }
        }

        NewLine();

        // Высота
        {
            TextUnformatted("Высота");

            SameLine();
            if (SliderInt("##font_height", &font_settings.height, 4, 120))
            {
                need_generate = true;
                last_interaction_time = now;
            }
            SetItemTooltip("В пикселях\n\nРеальная высота символов в текстуре может\nувеличиваться за счёт обводки и размытия");
        }

        // Антиалиасинг
        {
            TextUnformatted("Антиалиасинг");

            SameLine();
            if (Checkbox("##font_anti_aliasing", &font_settings.anti_aliasing))
            {
                need_generate = true;
                last_interaction_time = now;
            }
        }

        NewLine();

        // Стиль
        {
            TextUnformatted("Стиль");

            SameLine();
            const char* items[] = {"Простой", "С обводкой", "Только контур"};
            static_assert(IM_ARRAYSIZE(items) == (i32)FontStyle::last + 1);
            if (Combo("##style_idx", (i32*)&font_settings.font_style, items, IM_ARRAYSIZE(items)))
            {
                need_generate = true;
                last_interaction_time = now;
            }
        }

        // Простой стиль
        if (font_settings.font_style == FontStyle::simple)
        {
            Bullet();
            TextUnformatted("Цвет");
            SameLine();
            if (ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview))
            {
                need_generate = true;
                last_interaction_time = now;
            }

            Bullet();
            TextUnformatted("Радиус размытия");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                if (SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val))
                {
                    need_generate = true;
                    last_interaction_time = now;
                }
            }
        }
        // С обводкой
        else if (font_settings.font_style == FontStyle::outlined)
        {
            Bullet();
            TextUnformatted("Основной цвет");
            SameLine();
            if (ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview))
            {
                need_generate = true;
                last_interaction_time = now;
            }

            Bullet();
            TextUnformatted("Обводка");

            Indent(GetFontSize());

            Bullet();
            TextUnformatted("Цвет");
            SameLine();
            if (ColorEdit4("##second_color", (f32*)&font_settings.second_color, ImGuiColorEditFlags_AlphaPreview))\
            {
                need_generate = true;
                last_interaction_time = now;
            }

            Bullet();
            TextUnformatted("Толщина");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
                if (SliderInt("##thickness", &font_settings.thickness, min_val, max_val))
                {
                    need_generate = true;
                    last_interaction_time = now;
                }
            }

            Bullet();
            TextUnformatted("Радиус размытия");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                if (SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val))
                {
                    need_generate = true;
                    last_interaction_time = now;
                }
            }

            Unindent(GetFontSize());
        }
        // Только контур
        else if (font_settings.font_style == FontStyle::contour)
        {
            Bullet();
            TextUnformatted("Цвет");
            SameLine();
            if (ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview))
            {
                need_generate = true;
                last_interaction_time = now;
            }

            Bullet();
            TextUnformatted("Толщина");
            SameLine();
            {
                constexpr i32 min_val = 1;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
                if (SliderInt("##thickness", &font_settings.thickness, min_val, max_val))
                {
                    need_generate = true;
                    last_interaction_time = now;
                }
            }

            Bullet();
            TextUnformatted("Радиус размытия");
            SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                if (SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val))
                {
                    need_generate = true;
                    last_interaction_time = now;
                }
            }
        }

        NewLine();

        // Размер текстуры
        {
            TextUnformatted("Размер текстуры");

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

        End();
    }

    // Генерация
    {
        float idleTime = std::chrono::duration<float>(now - last_interaction_time).count();

        if (need_generate && idleTime > 0.5f) // секунд
        {
            if (generated_font_)
            {
                for (const shared_ptr<Texture>& texture : generated_font_->textures())
                {
                    DV_TEXTURE_CACHE->remove(texture);
                    assert(texture.use_count() == 1);
                }

                generated_font_ = nullptr;
            }

            if (font_settings.font_style == FontStyle::simple)
            {
                SFSettingsSimple sf_settings(font_settings.src_path,
                                             font_settings.height,
                                             font_settings.anti_aliasing,
                                             font_settings.blur_radius,
                                             (u32)ColorConvertFloat4ToU32(font_settings.main_color),
                                             font_settings.texture_size);

                // Измеренное тут время чуть больше, чем измеренное внутри конструктора SpriteFont
                auto begin_time = chrono::high_resolution_clock::now();
                generated_font_ = make_unique<SpriteFont>(sf_settings);
                auto end_time = chrono::high_resolution_clock::now();
                auto duration = end_time - begin_time;
                generation_time = (i64)chrono::duration_cast<chrono::milliseconds>(duration).count();

                ref_clamp(current_page, 0, (i32)generated_font_->textures().size() - 1);
            }
            else if (font_settings.font_style == FontStyle::contour)
            {
                SFSettingsContour sf_settings(font_settings.src_path,
                                              font_settings.height,
                                              font_settings.thickness,
                                              font_settings.anti_aliasing,
                                              font_settings.blur_radius,
                                              (u32)ColorConvertFloat4ToU32(font_settings.main_color),
                                              font_settings.texture_size);

                // Измеренное тут время чуть больше, чем измеренное внутри конструктора SpriteFont
                auto begin_time = chrono::high_resolution_clock::now();
                generated_font_ = make_unique<SpriteFont>(sf_settings);
                auto end_time = chrono::high_resolution_clock::now();
                auto duration = end_time - begin_time;
                generation_time = (i64)chrono::duration_cast<chrono::milliseconds>(duration).count();

                ref_clamp(current_page, 0, (i32)generated_font_->textures().size() - 1);
            }
            else if (font_settings.font_style == FontStyle::outlined)
            {
                 SFSettingsOutlined sf_settings(font_settings.src_path,
                                                font_settings.height,
                                                (u32)ColorConvertFloat4ToU32(font_settings.main_color),
                                                (u32)ColorConvertFloat4ToU32(font_settings.second_color),
                                                font_settings.thickness,
                                                font_settings.blur_radius,
                                                font_settings.anti_aliasing,
                                                font_settings.texture_size);

                // Измеренное тут время чуть больше, чем измеренное внутри конструктора SpriteFont
                auto begin_time = chrono::high_resolution_clock::now();
                generated_font_ = make_unique<SpriteFont>(sf_settings);
                auto end_time = chrono::high_resolution_clock::now();
                auto duration = end_time - begin_time;
                generation_time = (i64)chrono::duration_cast<chrono::milliseconds>(duration).count();

                ref_clamp(current_page, 0, (i32)generated_font_->textures().size() - 1);
            }

            need_generate = false;
            last_interaction_time = now;
        }
    }

    // Окно вывода текстур
    {
        SetNextWindowPos(ImVec2(520.f, 10.f), ImGuiCond_FirstUseEver);
        SetNextWindowSize(ImVec2(500.f, 344.f), ImGuiCond_FirstUseEver);
        Begin("Сгенерированный шрифт");

        if (generated_font_ && generated_font_->textures().size() > 0)
        {
            if (Button("<"))
            {
                --current_page;
                ref_clamp(current_page, 0, (i32)generated_font_->textures().size() - 1);
            }

            SameLine();
            Text("Страница: %d", current_page);

            SameLine();
            if (Button(">"))
            {
                ++current_page;
                ref_clamp(current_page, 0, (i32)generated_font_->textures().size() - 1);
            }

            // Число страниц и время генерации
            {
                StrUtf8 str = "Число страниц: " + to_string(generated_font_->textures().size());

                if (generation_time)
                    str += " | Время генерации: " + to_string(generation_time) + " мс";

                SameLine();
                TextUnformatted(str.c_str());
            }
        
            // Изображение
            {
                f32 child_height = GetContentRegionAvail().y - GetFrameHeightWithSpacing();
                BeginChild("ImageScrollArea", ImVec2(0, child_height), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
                Texture* texture = generated_font_->textures()[current_page].get();
                ImVec2 image_size((f32)texture->width(), (f32)texture->height());
                ImGui::Image(texture->gpu_object_name(), image_size);
                EndChild();
            }

            // Кнопка сохранения
            {
                // Располагаем кнопку справа
                const StrUtf8 save_button_label = "Сохранить";
                f32 width = calc_button_width(save_button_label);
                f32 avail = GetContentRegionAvail().x;
                f32 offset = avail - width;
                SetCursorPosX(GetCursorPosX() + offset);

                if (Button(save_button_label.c_str()))
                    file_dialog_state = FileDialogState::save_file;
            }
        }

        End();
    }

    preview_window_->show(generated_font_.get());

    // Файловый диалог
    {
        if (file_dialog_state == FileDialogState::open_file)
        {
            bool visible = true;

            if (open_file_dialog(visible, src_font_path_inside_dialog))
            {
                if (font_settings.src_path != src_font_path_inside_dialog)
                {
                    font_settings.src_path = src_font_path_inside_dialog;
                    need_generate = true;
                    last_interaction_time = now;
                }
            }

            if (!visible)
                file_dialog_state = FileDialogState::closed;
        }
        else if (file_dialog_state == FileDialogState::save_file)
        {
            bool visible = true;
            static StrUtf8 result_path = get_base_path() + "result.fnt";

            if (save_file_dialog(visible, result_path))
                generated_font_->save(result_path);

            if (!visible)
                file_dialog_state = FileDialogState::closed;
        }
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
