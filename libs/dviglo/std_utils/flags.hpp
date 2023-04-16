// Copyright (c) the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#pragma once

#include "../common/primitive_types.hpp"

#include <type_traits> // std::underlying_type_t


namespace dviglo
{

/// Определяет побитовые операции для scoped enum.
/// Используйте !! для преобразования scoped enum в bool
#define DV_FLAGS(EnumClass) \
    inline constexpr EnumClass operator |(const EnumClass lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return static_cast<EnumClass>(static_cast<Ut>(lhs) | static_cast<Ut>(rhs)); \
    } \
    inline constexpr EnumClass& operator |=(EnumClass& lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        lhs = static_cast<EnumClass>(static_cast<Ut>(lhs) | static_cast<Ut>(rhs)); \
        return lhs; \
    } \
    inline constexpr EnumClass operator &(const EnumClass lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return static_cast<EnumClass>(static_cast<Ut>(lhs) & static_cast<Ut>(rhs)); \
    } \
    inline constexpr EnumClass& operator &=(EnumClass& lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        lhs = static_cast<EnumClass>(static_cast<Ut>(lhs) & static_cast<Ut>(rhs)); \
        return lhs; \
    } \
    inline constexpr EnumClass operator ^(const EnumClass lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return static_cast<EnumClass>(static_cast<Ut>(lhs) ^ static_cast<Ut>(rhs)); \
    } \
    inline constexpr EnumClass& operator ^=(EnumClass& lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        lhs = static_cast<EnumClass>(static_cast<Ut>(lhs) ^ static_cast<Ut>(rhs)); \
        return lhs; \
    } \
    inline constexpr EnumClass operator ~(const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return static_cast<EnumClass>(~static_cast<Ut>(rhs)); \
    } \
    inline constexpr bool operator ==(const EnumClass lhs, const std::underlying_type_t<EnumClass> rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return static_cast<Ut>(lhs) == rhs; \
    } \
    inline constexpr bool operator ==(const std::underlying_type_t<EnumClass> lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return lhs == static_cast<Ut>(rhs); \
    } \
    inline constexpr bool operator !=(const EnumClass lhs, const std::underlying_type_t<EnumClass> rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return static_cast<Ut>(lhs) != rhs; \
    } \
    inline constexpr bool operator !=(const std::underlying_type_t<EnumClass> lhs, const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return lhs != static_cast<Ut>(rhs); \
    } \
    inline constexpr bool operator !(const EnumClass rhs) \
    { \
        using Ut = std::underlying_type_t<EnumClass>; \
        return static_cast<Ut>(rhs) == 0; \
    }

} // namespace dviglo
