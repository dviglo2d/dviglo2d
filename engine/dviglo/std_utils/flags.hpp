// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <type_traits> // std::underlying_type_t


namespace dviglo
{

// Определяет побитовые операции для scoped enum.
// Используйте !! для преобразования scoped enum в bool
#define DV_FLAGS(EnumClass)                                                                      \
    constexpr bool operator !(const EnumClass rhs)                                               \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        return !static_cast<U>(rhs);                                                             \
    }                                                                                            \
    constexpr EnumClass operator ~(const EnumClass rhs)                                          \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        return static_cast<EnumClass>(~static_cast<U>(rhs));                                     \
    }                                                                                            \
    constexpr bool operator ==(const EnumClass lhs, const std::underlying_type_t<EnumClass> rhs) \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        return static_cast<U>(lhs) == rhs;                                                       \
    }                                                                                            \
    constexpr EnumClass operator &(const EnumClass lhs, const EnumClass rhs)                     \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        return static_cast<EnumClass>(static_cast<U>(lhs) & static_cast<U>(rhs));                \
    }                                                                                            \
    constexpr EnumClass operator ^(const EnumClass lhs, const EnumClass rhs)                     \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        return static_cast<EnumClass>(static_cast<U>(lhs) ^ static_cast<U>(rhs));                \
    }                                                                                            \
    constexpr EnumClass operator |(const EnumClass lhs, const EnumClass rhs)                     \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        return static_cast<EnumClass>(static_cast<U>(lhs) | static_cast<U>(rhs));                \
    }                                                                                            \
    constexpr EnumClass& operator &=(EnumClass& lhs, const EnumClass rhs)                        \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        lhs = static_cast<EnumClass>(static_cast<U>(lhs) & static_cast<U>(rhs));                 \
        return lhs;                                                                              \
    }                                                                                            \
    constexpr EnumClass& operator ^=(EnumClass& lhs, const EnumClass rhs)                        \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        lhs = static_cast<EnumClass>(static_cast<U>(lhs) ^ static_cast<U>(rhs));                 \
        return lhs;                                                                              \
    }                                                                                            \
    constexpr EnumClass& operator |=(EnumClass& lhs, const EnumClass rhs)                        \
    {                                                                                            \
        using U = std::underlying_type_t<EnumClass>;                                             \
        lhs = static_cast<EnumClass>(static_cast<U>(lhs) | static_cast<U>(rhs));                 \
        return lhs;                                                                              \
    }

} // namespace dviglo
