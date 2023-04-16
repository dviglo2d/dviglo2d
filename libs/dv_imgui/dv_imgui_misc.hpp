// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_string.hpp>
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

enum class FileDialogState
{
    closed,
    open_file,
    save_file,
    //choose_dir, // TODO
};

// Возвращает true при нажатии кнопки "Открыть" или "Сохранить" (в зависимости от типа диалога)
bool file_dialog(FileDialogState& state, StrUtf8& path);

} // namespace dviglo
