// Copyright (c) the Dviglo project
// License: MIT

// Функции, которые можно использовать до инициализации любых подсистем

#pragma once

#include "../std_utils/str.hpp"


namespace dviglo
{

bool dir_exists(StrViewUtf8 path);

/// Версия функции, которая не пишет с лог
bool create_dir_silent(StrViewUtf8 path);

/// Аналог SDL_GetPrefPath(), который не требует инициализации SDL
/// <https://github.com/libsdl-org/SDL/issues/2587>.
/// org может быть "".
/// В конце пути добавляет '/'.
/// В случае неудачи возвращает пустую строку
StrUtf8 get_pref_path(StrViewUtf8 org, StrViewUtf8 app);

/// Аналог SDL_GetBasePath(), который не требует инициализации SDL
/// <https://github.com/libsdl-org/SDL/issues/2587>.
/// В конце пути добавляет '/'.
/// В случае неудачи возвращает пустую строку
StrUtf8 get_base_path();

} // namespace dviglo
