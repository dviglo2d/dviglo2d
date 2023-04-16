// Copyright (c) the Dviglo project
// License: MIT

// Функции для работы с файловыми путями.
// Можно использовать до инициализации любых подсистем

#pragma once

#include "../std_utils/string.hpp"


namespace dviglo
{

/// Заменяет '\\' на '/'
constexpr StrUtf8 to_internal(StrViewUtf8 path)
{
    return replace_all(path, '\\', '/');
}

/// В Windows заменяет '/' на '\\'
constexpr StrUtf8 to_native(const StrUtf8& path)
{
#ifdef _WIN32
    return replace_all(path, '/', '\\');
#else
    return path;
#endif
}

#ifdef _WIN32
/// В Windows заменяет '/' на '\\' и преобразует в кодировку UTF-16.
/// В Linux для путей используется кодировка UTF-8, поэтому преобразование не требуется
constexpr std::wstring to_win_native(StrViewUtf8 path)
{
    return to_wstring(replace_all(path, '/', '\\'));
}
#endif

/// Удаляет один '/' в конце строки, если он есть
constexpr StrUtf8 trim_end_slash(StrViewUtf8 path)
{
    StrUtf8 ret(path);

    if (!ret.empty() && ret.back() == '/')
        ret.resize(ret.length() - 1);

    return ret;
}

/// Возвращает родительский путь (с / в конце) или пустую строку
constexpr StrUtf8 get_parent(StrViewUtf8 path)
{
    size_t pos = trim_end_slash(path).find_last_of('/');

    if (pos != StrUtf8::npos)
        return StrUtf8(path.substr(0, pos + 1));
    else
        return StrUtf8();
}

} // namespace dviglo
