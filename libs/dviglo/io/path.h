// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

// Функции для работы с файловыми путями.
// Можно использовать до инициализации любых подсистем

#pragma once

#include "../std_utils/str.h"


namespace dviglo
{

/// Заменяет '\\' на '/'
constexpr std::string to_internal(std::string_view utf8_path)
{
    return replace_all(utf8_path, '\\', '/');
}

/// В Windows заменяет '/' на '\\'
constexpr std::string to_native(std::string_view utf8_path)
{
#ifdef _WIN32
    return replace_all(utf8_path, '/', '\\');
#else
    return std::string(utf8_path);
#endif
}

#ifdef _WIN32
/// В Windows заменяет '/' на '\\' и преобразует в кодировку UTF-16.
/// В Linux для путей используется кодировка UTF-8, поэтому преобразование не требуется
constexpr std::wstring to_win_native(std::string_view utf8_path)
{
    return to_wstring(replace_all(utf8_path, '/', '\\'));
}
#endif

/// Удаляет один '/' в конце строки, если он есть
constexpr std::string trim_end_slash(std::string_view utf8_path)
{
    std::string ret(utf8_path);

    if (!ret.empty() && ret.back() == '/')
        ret.resize(ret.length() - 1);

    return ret;
}

/// Возвращает родительский путь (с / в конце) или пустую строку
constexpr std::string get_parent(std::string_view utf8_path)
{
    size_t pos = trim_end_slash(utf8_path).find_last_of('/');

    if (pos != std::string::npos)
        return std::string(utf8_path.substr(0, pos + 1));
    else
        return std::string();
}

} // namespace dviglo
