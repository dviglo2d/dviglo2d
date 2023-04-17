// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

// Файл называется str.h, а не string.h, так как в стандартной библиотеке
// уже есть файл string.h

#pragma once

#include <string>


namespace dviglo
{

/// \todo Удалить при переходе на C++23
inline constexpr bool contains(const std::string& str, char value) noexcept
{
    return str.find(value) != std::string::npos;
}

/// \todo Удалить при переходе на C++23
inline constexpr bool contains(const std::string& str, const char* value) noexcept
{
    return str.find(value) != std::string::npos;
}

/// \todo Удалить при переходе на C++23
inline constexpr bool contains(const std::string& str, std::string_view value) noexcept
{
    return str.find(value) != std::string::npos;
}

} // namespace dviglo
