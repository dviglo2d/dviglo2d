// Copyright (c) the Dviglo project
// License: MIT

#include "dv_imgui_misc.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>


namespace dviglo
{

bool file_dialog(FileDialogState& state, StrUtf8& path)
{
    namespace ig = ImGui;

    if (state == FileDialogState::closed)
        return false;

    const char* title = (state == FileDialogState::save_file) ? "Сохранение файла"
        : (state == FileDialogState::open_file) ? "Открытие файла" : "Выбор папки";

    if (!ig::IsPopupOpen(title))
        ig::OpenPopup(title);

    bool ret = false;
    bool open = true; // Отлавливаем нажатие на крестик

    ig::SetNextWindowPos(ImVec2(100.f, 100.f), ImGuiCond_FirstUseEver);
    ig::SetNextWindowSize(ImVec2(500.f, 500.f), ImGuiCond_FirstUseEver);

    if (ig::BeginPopupModal(title, &open, ImGuiWindowFlags_NoCollapse))
    {
        // Список файлов
        {
            // Оставляем внизу пространство для кнопок
            f32 child_height = ig::GetContentRegionAvail().y - ig::GetFrameHeightWithSpacing();

            ig::BeginChild("##files", ImVec2(0, child_height), ImGuiChildFlags_Borders);
            ig::TextUnformatted("TODO");
            ig::EndChild();
        }

        const StrUtf8 action_button_label = (state == FileDialogState::save_file) ? "Сохранить"
            : (state == FileDialogState::open_file) ? "Открыть" : "Выбрать";

        const StrUtf8 cancel_button_label = "Отмена";
        const f32 button_width = std::max(calc_button_width(action_button_label), calc_button_width(cancel_button_label));

        // Текстовое поле с путём
        {
            ig::PushItemWidth(-button_width * 2 - ig::GetStyle().ItemSpacing.x * 2); // Label не нужен
            ig::InputText("##path", &path, ImGuiInputTextFlags_ElideLeft);
            ig::PopItemWidth();
        }

        // Две кнопки
        {
            ig::SameLine();
            if (ig::Button(action_button_label.c_str(), ImVec2(button_width, 0)))
            {
                ret = true;
                state = FileDialogState::closed;
            }

            ig::SameLine();
            if (ig::Button(cancel_button_label.c_str(), ImVec2(button_width, 0)))
                state = FileDialogState::closed;
        }

        ig::EndPopup();
    }

    if (!open) // Пользователь нажал на крестик
        state = FileDialogState::closed;

    return ret;
}

} // namespace dviglo
