#pragma once

#include <dviglo/res/sprite_font.hpp>

#include <dv_imgui_misc.hpp>

using namespace dviglo;


void show_result_window(const SpriteFont* font, const i64 generation_time_ms, FileDialogState& file_dialog_state);

void show_save_dialog(const SpriteFont* font, FileDialogState& state);
