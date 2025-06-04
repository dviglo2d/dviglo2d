// Copyright (c) the Dviglo project
// License: MIT

// Функции, которые можно использовать до инициализации любых подсистем (не пишут в лог)

#pragma once

#include "path.hpp"


namespace dviglo
{

inline bool dir_exists(const std::filesystem::path& path) noexcept
{
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

// Создаёт папку и все родительские папки.
// В MSVC функция create_directories(...) всегда возвращает
 // false, если в конце пути стоит /
inline bool create_dir_silent(const std::filesystem::path& path) noexcept
{
    // В отличие от GCC в MSVC () функция create_directories(...) всегда возвращает
    // false, если в конце пути стоит /
    std::filesystem::path fixed_path = path;


    std::error_code ec;

    std::filesystem::create_directories(path.parent_path(), ec);

    if (ec)
        return false;

    return std::filesystem::is_directory(path, ec);
}

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
