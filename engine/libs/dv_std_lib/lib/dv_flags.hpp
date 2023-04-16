// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <utility> // std::to_underlying(...)


namespace dviglo
{

// Определяет побитовые операции для scoped enum.
// Используйте !! для преобразования scoped enum в bool
#define DV_FLAGS(EnumClass)                                                                                  \
    [[nodiscard]] constexpr bool operator !(EnumClass rhs) noexcept                                          \
    {                                                                                                        \
        return !std::to_underlying(rhs);                                                                     \
    }                                                                                                        \
    [[nodiscard]] constexpr EnumClass operator ~(EnumClass rhs) noexcept                                     \
    {                                                                                                        \
        return static_cast<EnumClass>(~std::to_underlying(rhs));                                             \
    }                                                                                                        \
    [[nodiscard]] constexpr bool operator ==(EnumClass lhs, std::underlying_type_t<EnumClass> rhs) noexcept  \
    {                                                                                                        \
        return std::to_underlying(lhs) == rhs;                                                               \
    }                                                                                                        \
    [[nodiscard]] constexpr EnumClass operator &(EnumClass lhs, EnumClass rhs) noexcept                      \
    {                                                                                                        \
        return static_cast<EnumClass>(std::to_underlying(lhs) & std::to_underlying(rhs));                    \
    }                                                                                                        \
    [[nodiscard]] constexpr EnumClass operator ^(EnumClass lhs, EnumClass rhs) noexcept                      \
    {                                                                                                        \
        return static_cast<EnumClass>(std::to_underlying(lhs) ^ std::to_underlying(rhs));                    \
    }                                                                                                        \
    [[nodiscard]] constexpr EnumClass operator |(EnumClass lhs, EnumClass rhs) noexcept                      \
    {                                                                                                        \
        return static_cast<EnumClass>(std::to_underlying(lhs) | std::to_underlying(rhs));                    \
    }                                                                                                        \
    constexpr EnumClass& operator &=(EnumClass& lhs, EnumClass rhs) noexcept                                 \
    {                                                                                                        \
        return lhs = lhs & rhs;                                                                              \
    }                                                                                                        \
    constexpr EnumClass& operator ^=(EnumClass& lhs, EnumClass rhs) noexcept                                 \
    {                                                                                                        \
        return lhs = lhs ^ rhs;                                                                              \
    }                                                                                                        \
    constexpr EnumClass& operator |=(EnumClass& lhs, EnumClass rhs) noexcept                                 \
    {                                                                                                        \
        return lhs = lhs | rhs;                                                                              \
    }

} // namespace dviglo
