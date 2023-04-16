// Copyright (c) the Dviglo project
// License: MIT

// Функции, которые можно использовать до инициализации любых подсистем (не пишут в лог)

#pragma once

#include "../std_utils/fs.hpp"
#include "../std_utils/string.hpp"


namespace dviglo
{

// Аналог SDL_GetPrefPath(), который не требует инициализации SDL
// <https://github.com/libsdl-org/SDL/issues/2587>.
// org может быть "".
// Автоматически создаёт папки.
// В случае неудачи возвращает пустой путь
fs::path get_pref_path(StrViewUtf8 org, StrViewUtf8 app);

// Путь к исполняемому файлу.
// В случае неудачи возвращает пустой путь
fs::path get_exe_path();

// Аналог SDL_GetBasePath(), который не требует инициализации SDL
// <https://github.com/libsdl-org/SDL/issues/2587>.
// В случае неудачи возвращает пустой путь
inline fs::path get_base_path()
{
    return get_exe_path().parent_path();
}

} // namespace dviglo
