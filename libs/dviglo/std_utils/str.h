// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

// Файл называется str.h, а не string.h, так как в стандартной библиотеке
// уже есть файл string.h

#pragma once

#include "../common/config.h"

#include <string>


namespace dviglo
{

/// \todo Удалить при переходе на C++23
constexpr bool contains(const std::string& str, char value) noexcept
{
    return str.find(value) != std::string::npos;
}

/// \todo Удалить при переходе на C++23
constexpr bool contains(const std::string& str, const char* value) noexcept
{
    return str.find(value) != std::string::npos;
}

/// \todo Удалить при переходе на C++23
constexpr bool contains(const std::string& str, std::string_view value) noexcept
{
    return str.find(value) != std::string::npos;
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

} // namespace dviglo
