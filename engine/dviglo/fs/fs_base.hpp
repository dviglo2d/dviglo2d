// Copyright (c) the Dviglo project
// License: MIT

// Функции, которые можно использовать до инициализации любых подсистем (не пишут в лог)

#pragma once

#include "path.hpp"


namespace dviglo
{

// Концепт запрещает функции принимать строку
template<std::same_as<std::filesystem::path> Path>
bool dir_exists(const Path& path)
{
    std::error_code ec;
    return std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec);
}

inline bool dir_exists(StrViewUtf8 path)
{
    return dir_exists(to_path(path));
}

// Версия функции, которая не пишет с лог
bool create_dir_silent(const StrUtf8& path);

// Аналог SDL_GetPrefPath(), который не требует инициализации SDL
// <https://github.com/libsdl-org/SDL/issues/2587>.
// org может быть "".
// В конце пути добавляет '/'.
// Автоматически создаёт папки.
// В случае неудачи возвращает пустую строку
StrUtf8 get_pref_path(StrViewUtf8 org, StrViewUtf8 app);

// Аналог SDL_GetBasePath(), который не требует инициализации SDL
// <https://github.com/libsdl-org/SDL/issues/2587>.
// В конце пути добавляет '/'.
// В случае неудачи возвращает пустую строку
StrUtf8 get_base_path();

} // namespace dviglo
