#include "dv_imgui_utils.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>


namespace dviglo
{

enum class DialogType
{
    open_file,
    save_file,
    choose_dir, // TODO
};

static bool dialog(const DialogType dialog_type, bool& visible, StrUtf8& path)
{
    if (!visible)
        return false;

    namespace ig = ImGui;

    ig::SetNextWindowPos(ImVec2(100.f, 100.f), ImGuiCond_FirstUseEver);
    ig::SetNextWindowSize(ImVec2(500.f, 500.f), ImGuiCond_FirstUseEver);
    ig::Begin("Сохранение файла", &visible, ImGuiWindowFlags_NoCollapse);

    // Список файлов
    {
        // Оставляем внизу пространство для кнопок
        f32 child_height = ig::GetContentRegionAvail().y - ig::GetFrameHeightWithSpacing();

        ig::BeginChild("##files", ImVec2(0, child_height), ImGuiChildFlags_Border);
        ig::TextUnformatted("TODO");
        ig::EndChild();
    }

    const StrUtf8 action_button_label = (dialog_type == DialogType::save_file) ? "Сохранить"
        : (dialog_type == DialogType::open_file) ? "Открыть" : "Выбрать";

    const StrUtf8 cancel_button_label = "Отмена";
    const f32 button_width = std::max(calc_button_width(action_button_label), calc_button_width(cancel_button_label));

    // Текстовое поле с путём
    {
        ig::PushItemWidth(-button_width * 2 - ig::GetStyle().ItemSpacing.x * 2); // Label не нужен
        ig::InputText("##path", &path, ImGuiInputTextFlags_ElideLeft);
        ig::PopItemWidth();
    }

    bool ret = false;

    // Две кнопки
    {
        ig::SameLine();
        if (ig::Button(action_button_label.c_str(), ImVec2(button_width, 0)))
        {
            ret = true;
            visible = false;
        }

        ig::SameLine();
        if (ImGui::Button(cancel_button_label.c_str(), ImVec2(button_width, 0)))
            visible = false;
    }

    ig::End();

    return ret;
}

bool open_file_dialog(bool& visible, StrUtf8& path)
{
    return dialog(DialogType::open_file, visible, path);
}

bool save_file_dialog(bool& visible, StrUtf8& path)
{
    return dialog(DialogType::save_file, visible, path);
}

} // namespace dviglo
