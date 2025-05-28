// Copyright (c) the Dviglo project
// License: MIT

// Функции, которые можно использовать до инициализации любых подсистем (не пишут в лог)

#pragma once

#include "path.hpp"


namespace dviglo
{

// Концепт запрещает функции принимать строку
template <std::same_as<std::filesystem::path> Path>
bool dir_exists(const Path& path) noexcept
{
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

inline bool dir_exists(StrViewUtf8 path) noexcept
{
    return dir_exists(std::filesystem::path(path));
}

// Создаёт папку и все родительские папки.
// Возвращает true, если папка создана или уже существует.
// Концепт запрещает функции принимать строку
template <std::same_as<std::filesystem::path> Path>
bool create_dir_silent(const Path& path) noexcept
{
    std::error_code ec;

    // В данном случае это не важно, но в MSVC (в отличие от GCC) функция всегда возвращает
    // false, если в конце пути стоит /
    std::filesystem::create_directories(path, ec);

    if (ec)
        return false;

    return std::filesystem::is_directory(path, ec);
}

// Создаёт папку и все родительские папки.
// Возвращает true, если папка создана или уже существует
inline bool create_dir_silent(const StrUtf8& path) noexcept
{
    return create_dir_silent(std::filesystem::path(path));
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
