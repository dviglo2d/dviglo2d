// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

// Файл называется str.h, а не string.h, так как в стандартной библиотеке
// уже есть файл string.h

#pragma once

#include "../common/config.h"

#include <algorithm>
#include <string>


namespace dviglo
{

constexpr bool contains(std::string_view str, char c) noexcept
{
    return str.find(c) != std::string::npos;
}

constexpr bool contains(std::string_view str, std::string_view substr) noexcept
{
    return str.find(substr) != std::string::npos;
}

constexpr std::string replace_all(std::string_view str, std::string_view old_substr, std::string_view new_substr)
{
    std::string ret;

    std::size_t offset = 0;
    std::size_t pos = str.find(old_substr); // Позиция old_value в исходной строке

    while (pos != std::string::npos)
    {
        ret.append(str, offset, pos - offset); // Копируем фрагмент до найденной подстроки
        ret += new_substr;
        offset = pos + old_substr.length(); // Смещение после найденной подстроки
        pos = str.find(old_substr, offset);
    }

    ret += str.substr(offset); // Копируем остаток строки

    return ret;
}

/// В отличие от std::tolower() может работать в compile time
constexpr char to_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

/// В отличие от std::toupper() может работать в compile time
constexpr char to_upper(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

// Работает только для ASCII-строк
constexpr std::string replace_all(std::string_view str, char old_c, char new_c, bool case_sensitive = true)
{
    std::string ret(str);

    if (case_sensitive)
    {
        std::replace(ret.begin(), ret.end(), old_c, new_c);
    }
    else
    {
        old_c = to_lower(old_c);

        for (size_t i = 0; i < str.length(); ++i)
        {
            if (to_lower(ret[i]) == old_c)
                ret[i] = new_c;
        }
    }

    return ret;
}

} // namespace dviglo
