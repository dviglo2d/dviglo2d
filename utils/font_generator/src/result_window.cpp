#include "result_window.hpp"

#include <cinttypes> // PRId64
#include <dv_imgui_misc.hpp>
#include <dv_math.hpp>

using namespace std;


void show_result_window(const SpriteFont* font, const i64 generation_time_ms, FileDialogState& file_dialog_state)
{
    namespace ig = ImGui;

    static i32 current_atals_index = 0;

    ig::SetNextWindowPos(ImVec2(520.f, 10.f), ImGuiCond_FirstUseEver);
    ig::SetNextWindowSize(ImVec2(500.f, 428.f), ImGuiCond_FirstUseEver);
    if (ig::Begin("Сгенерированный шрифт"))
    {
        if (font && font->textures().size() > 0)
        {
            ref_clamp(current_atals_index, 0, (i32)font->textures().size() - 1);

            if (ig::Button("<"))
            {
                --current_atals_index;
                ref_clamp(current_atals_index, 0, (i32)font->textures().size() - 1);
            }

            ig::SameLine();
            ig::Text("Атлас: %d", current_atals_index);

            ig::SameLine();
            if (ig::Button(">"))
            {
                ++current_atals_index;
                ref_clamp(current_atals_index, 0, (i32)font->textures().size() - 1);
            }

            // Число атласов
            {
                ig::SameLine();
                const StrUtf8 str = "Число атласов: " + to_string(font->textures().size());
                ig::TextUnformatted(str.c_str());
            }

            // Изображение
            {
                const f32 child_height = ig::GetContentRegionAvail().y - ig::GetFrameHeightWithSpacing();
                if (ig::BeginChild("ImageScrollArea", ImVec2(0, child_height), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    const Texture* texture = font->textures()[current_atals_index].get();
                    const ImVec2 image_size((f32)texture->width(), (f32)texture->height());
                    ImGui::Image(texture->gpu_object_name(), image_size);
                }
                ig::EndChild(); // Нужно вызывать даже если BeginChild() вернул false
            }

            ig::Text("Время генерации: %" PRId64 " мс", generation_time_ms);

            // Кнопка сохранения
            {
                ig::SameLine();

                // Располагаем кнопку справа
                const StrUtf8 save_button_label = "Сохранить";
                const f32 offset = ig::GetContentRegionAvail().x - calc_button_width(save_button_label);
                ig::SetCursorPosX(ig::GetCursorPosX() + offset);

                if (ig::Button(save_button_label.c_str()))
                    file_dialog_state = FileDialogState::save_file;
            }
        }
    }
    ig::End(); // Нужно вызывать даже если Begin() вернул false
}

void show_save_dialog(const SpriteFont* font, FileDialogState& state)
{
    if (state != FileDialogState::save_file)
        return;

    static StrUtf8 result_path = (get_base_path() / "result.fnt").string();

    if (file_dialog(state, result_path))
        font->save(result_path);
}
