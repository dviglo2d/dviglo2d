#pragma once

#include <dviglo/std_utils/string.hpp>
#include <glm/glm.hpp>
#include <imgui.h>


namespace dviglo
{

inline f32 calc_button_width(const StrUtf8& label)
{
    f32 label_width = ImGui::CalcTextSize(label.c_str()).x;
    return label_width + ImGui::GetStyle().FramePadding.x * 2.f;
}

inline ImVec2 to_imvec2(const glm::ivec2 val)
{
    return ImVec2((f32)val.x, (f32)val.y);
}

// Возвращает true при нажатии кнопки "Открыть"
bool open_file_dialog(bool& visible, StrUtf8& path);

// Возвращает true при нажатии кнопки "Сохранить"
bool save_file_dialog(bool& visible, StrUtf8& path);

} // namespace dviglo
