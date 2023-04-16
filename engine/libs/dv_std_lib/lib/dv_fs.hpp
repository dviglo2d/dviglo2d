// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_string.hpp"

#include <filesystem>
#include <format>


namespace std
{

// Добавляем поддержку std::filesystem::path к std::format(...)
template <>
struct formatter<filesystem::path> : formatter<string>
{
    auto format(const filesystem::path& p, format_context& ctx) const
    {
        return formatter<string>::format(p.string(), ctx);
    }
};

}


namespace dviglo
{

namespace fs = std::filesystem;

// Возвращает расширение без точки
inline fs::path get_ext(const fs::path& path)
{
    fs::path ret = path.extension();

    // Убираем точку в начале
    if (ret.native().size())
        ret = fs::path(ret.native().substr(1));

    return ret;
}

inline bool dir_exists(const fs::path& path) noexcept
{
    std::error_code ec;
    return fs::is_directory(path, ec);
}

inline bool create_dirs(const fs::path& path) noexcept
{
    if (dir_exists(path))
        return false;

    // В MSVC функция create_directories(...) всегда возвращает false, если в конце пути стоит /
    // https://github.com/microsoft/STL/issues/736
    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec)
        return false;

    return true;
}

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
