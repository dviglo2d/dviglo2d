#include "settings_window.hpp"

#include <dv_imgui_misc.hpp>
#include <dv_math.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/main/timer.hpp>
#include <misc/cpp/imgui_stdlib.h>

using namespace glm;


enum class FontStyle : u32
{
    simple = 0,       // Обычный
    contour,          // Только контур
    outlined,         // С обводкой
    last = outlined   // Для определения числа стилей
};

struct FontSettings
{
    StrUtf8 src_path = (get_base_path() / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf").string();
    i32 height = 40; // В пикселях
    bool anti_aliasing = true;
    FontStyle font_style = FontStyle::simple;
    ImVec4 main_color{1.f, 1.f, 1.f, 1.f};   // Основной цвет
    ImVec4 second_color{0.f, 0.f, 0.f, 1.f}; // Дополнительный цвет
    f32 thickness = 2; // Толщина контура или обводки
    i32 blur_radius = 0;  // Радиус размытия
    bool render_inner = true; // Рендерить ли внутренний глиф (в стиле "с обводкой")
    ivec2 texture_size{1024, 1024};
};

static FontSettings font_settings;
static StrUtf8 src_font_path_inside_open_dialog;

void show_settings_window(unique_ptr<SpriteFont>& font,
                          i64& generation_time_ms,
                          i64& idle_time_ns,
                          FileDialogState& file_dialog_state)
{
    namespace ig = ImGui;

    ImGuiStyle& style = ig::GetStyle();

    ig::SetNextWindowPos(ImVec2(10.f, 10.f), ImGuiCond_FirstUseEver);
    const f32 window_width = 500.f;
    ig::SetNextWindowSizeConstraints(ImVec2(window_width, 0.f), ImVec2(window_width, FLT_MAX));
    if (ig::Begin("Настройки шрифта", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ig::PushItemWidth(-FLT_MIN); // Label элементов не используем

        // Исходный шрифт
        {
            ig::TextUnformatted("Исходный шрифт");

            const StrUtf8 explore_button_label = "...";
            ig::PushStyleVar(ImGuiStyleVar_Alpha, style.Alpha * style.DisabledAlpha); // Делаем текст серым
            ig::PushItemWidth(-calc_button_width(explore_button_label) - style.ItemSpacing.x);
            ig::InputText("##src_font_path", &font_settings.src_path, ImGuiInputTextFlags_ElideLeft | ImGuiInputTextFlags_ReadOnly);
            ig::PopItemWidth();
            ImGui::PopStyleVar();

            ig::SameLine();
            if (ig::Button(explore_button_label.c_str()))
            {
                file_dialog_state = FileDialogState::open_file;
                src_font_path_inside_open_dialog = font_settings.src_path;
            }
        }

        ig::NewLine();

        // Высота
        {
            ig::TextUnformatted("Высота");

            ig::SameLine();
            if (ig::SliderInt("##font_height", &font_settings.height, 4, 120))
                idle_time_ns = 0;
            ig::SetItemTooltip("В пикселях\n\nРеальная высота символов в текстуре чаще всего отличается");
        }

        // Антиалиасинг
        {
            ig::TextUnformatted("Антиалиасинг");

            ig::SameLine();
            if (ig::Checkbox("##font_anti_aliasing", &font_settings.anti_aliasing))
                idle_time_ns = 0;
        }

        ig::NewLine();

        // Стиль
        {
            ig::TextUnformatted("Стиль");

            ig::SameLine();
            const char* items[] = {"Обычный", "Только контур", "С обводкой"};
            static_assert(IM_ARRAYSIZE(items) == (i32)FontStyle::last + 1);
            if (ig::Combo("##style_idx", (i32*)&font_settings.font_style, items, IM_ARRAYSIZE(items)))
                idle_time_ns = 0;
        }

        // Обычный стиль
        if (font_settings.font_style == FontStyle::simple)
        {
            ig::Bullet();
            ig::TextUnformatted("Цвет");
            ig::SameLine();
            if (ig::ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview))
                idle_time_ns = 0;

            ig::Bullet();
            ig::TextUnformatted("Радиус размытия");
            ig::SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                if (ig::SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val))
                    idle_time_ns = 0;
            }
        }
        // Только контур
        else if (font_settings.font_style == FontStyle::contour)
        {
            ig::Bullet();
            ig::TextUnformatted("Цвет");
            ig::SameLine();
            if (ig::ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview))
                idle_time_ns = 0;

            ig::Bullet();
            ig::TextUnformatted("Толщина");
            ig::SameLine();
            {
                constexpr f32 min_val = 1.f / 64.f;
                constexpr f32 max_val = 20.f;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
                if (ig::InputFloat("##thickness", &font_settings.thickness, 1.f / 64.f, 1.f))
                    idle_time_ns = 0;
                // И снова ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
            }

            ig::Bullet();
            ig::TextUnformatted("Радиус размытия");
            ig::SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                if (ig::SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val))
                    idle_time_ns = 0;
            }
        }
        // С обводкой
        else if (font_settings.font_style == FontStyle::outlined)
        {
            ig::Bullet();
            ig::TextUnformatted("Основной цвет");
            ig::SameLine();
            ig::PushItemWidth(-ig::GetFrameHeight() - style.ItemSpacing.x); // Ширина Checkbox равна высоте
            if (ig::ColorEdit4("##main_color", (f32*)&font_settings.main_color, ImGuiColorEditFlags_AlphaPreview))
                idle_time_ns = 0;
            ig::PopItemWidth();

            ig::SameLine();
            if (ig::Checkbox("##render_inner", &font_settings.render_inner))
                idle_time_ns = 0;
            ig::SetItemTooltip("Рендерить ли внутренний глиф\n\nОтключить, если нужна размытая тень для текста с обводкой");

            ig::Bullet();
            ig::TextUnformatted("Обводка");

            ig::Indent(ig::GetFontSize());

            ig::Bullet();
            ig::TextUnformatted("Цвет");
            ig::SameLine();
            if (ig::ColorEdit4("##second_color", (f32*)&font_settings.second_color, ImGuiColorEditFlags_AlphaPreview))\
                idle_time_ns = 0;

            ig::Bullet();
            ig::TextUnformatted("Толщина");
            ig::SameLine();
            {
                constexpr f32 min_val = 0.f;
                constexpr f32 max_val = 20.f;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
                if (ig::InputFloat("##thickness", &font_settings.thickness, 1.f / 64.f, 1.f))
                    idle_time_ns = 0;
                // И снова ограничиваем
                ref_clamp(font_settings.thickness, min_val, max_val);
            }

            ig::Bullet();
            ig::TextUnformatted("Радиус размытия");
            ig::SameLine();
            {
                constexpr i32 min_val = 0;
                constexpr i32 max_val = 20;
                // Переменная используется в других стилях, ограничиваем
                ref_clamp(font_settings.blur_radius, min_val, max_val);
                if (ig::SliderInt("##blur_radius", &font_settings.blur_radius, min_val, max_val))
                    idle_time_ns = 0;
            }

            ig::Unindent(ig::GetFontSize());
        }

        ig::NewLine();

        // Размер текстуры
        {
            ig::TextUnformatted("Размер текстуры");

            const vector<i32> sizes{256, 512, 1024, 2048};

            ig::SameLine();
            f32 combo_width = (ig::GetContentRegionAvail().x - style.ItemSpacing.x) * 0.5f;
            ig::PushItemWidth(-combo_width - style.ItemSpacing.x);
            if (ig::BeginCombo("##texture_width", to_string(font_settings.texture_size.x).c_str()))
            {
                for (i32 size : sizes)
                {
                    const bool is_selected = (size == font_settings.texture_size.x);
                    if (ig::Selectable(to_string(size).c_str(), is_selected))
                    {
                        if (font_settings.texture_size.x != size)
                        {
                            font_settings.texture_size.x = size;
                            idle_time_ns = 0;
                        }
                    }

                    if (is_selected)
                        ig::SetItemDefaultFocus();
                }
                ig::EndCombo();
            }
            ig::SetItemTooltip("Ширина");
            ig::PopItemWidth();

            ig::SameLine();
            if (ig::BeginCombo("##texture_height", to_string(font_settings.texture_size.y).c_str()))
            {
                for (i32 size : sizes)
                {
                    const bool is_selected = (size == font_settings.texture_size.y);
                    if (ig::Selectable(to_string(size).c_str(), is_selected))
                    {
                        if (font_settings.texture_size.y != size)
                        {
                            font_settings.texture_size.y = size;
                            idle_time_ns = 0;
                        }
                    }

                    if (is_selected)
                        ig::SetItemDefaultFocus();
                }
                ig::EndCombo();
            }
            ig::SetItemTooltip("Высота");
        }

        ig::PopItemWidth(); // Label элементов не используем
    }
    ig::End(); // Нужно вызывать даже если Begin() вернул false

    // Генерация
    {
        if (idle_time_ns > ns_per_s)
        {
            // Уничтожаем шрифт и текстуры в кэше
            if (font)
            {
                for (const shared_ptr<Texture>& texture : font->textures())
                {
                    DV_TEXTURE_CACHE->remove(texture);
                    assert(texture.use_count() == 1);
                }

                font = nullptr;
            }

            if (font_settings.font_style == FontStyle::simple)
            {
                SFSettingsSimple sf_settings
                (
                    font_settings.src_path,
                    font_settings.height,
                    font_settings.anti_aliasing,
                    font_settings.blur_radius,
                    (u32)ig::ColorConvertFloat4ToU32(font_settings.main_color),
                    font_settings.texture_size
                );

                font = make_unique<SpriteFont>(sf_settings, &generation_time_ms);
            }
            else if (font_settings.font_style == FontStyle::contour)
            {
                SFSettingsContour sf_settings
                (
                    font_settings.src_path,
                    font_settings.height,
                    font_settings.thickness,
                    font_settings.anti_aliasing,
                    font_settings.blur_radius,
                    (u32)ig::ColorConvertFloat4ToU32(font_settings.main_color),
                    font_settings.texture_size
                );

                font = make_unique<SpriteFont>(sf_settings, &generation_time_ms);
            }
            else if (font_settings.font_style == FontStyle::outlined)
            {
                SFSettingsOutlined sf_settings
                (
                    font_settings.src_path,
                    font_settings.height,
                    (u32)ig::ColorConvertFloat4ToU32(font_settings.main_color),
                    (u32)ig::ColorConvertFloat4ToU32(font_settings.second_color),
                    font_settings.thickness,
                    font_settings.blur_radius,
                    font_settings.render_inner,
                    font_settings.anti_aliasing,
                    font_settings.texture_size
                );

                font = make_unique<SpriteFont>(sf_settings, &generation_time_ms);
            }

            idle_time_ns = -1;
        }
    }
}

void show_open_dialog(i64& idle_time_ns, FileDialogState& state)
{
    if (state != FileDialogState::open_file)
        return;

    if (file_dialog(state, src_font_path_inside_open_dialog))
    {
        if (font_settings.src_path != src_font_path_inside_open_dialog)
        {
            font_settings.src_path = src_font_path_inside_open_dialog;
            idle_time_ns = 0;
        }
    }
}
