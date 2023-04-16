#pragma once

#include <dviglo/res/sprite_font.hpp>

#include <dv_imgui_misc.hpp>

using namespace dviglo;
using namespace std;


void show_settings_window(unique_ptr<SpriteFont>& font,
                          i64& generation_time_ms,
                          i64& idle_time_ns,
                          FileDialogState& file_dialog_state);

void show_open_dialog(i64& idle_time_ns, FileDialogState& state);
