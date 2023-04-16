// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <bit>     // std::bit_cast
#include <utility> // std::forward


namespace dviglo
{

template<typename To, typename From>
constexpr To c_cast(From&& from)
{
    return const_cast<To>(std::forward<From>(from));
}

template<typename To, typename From>
constexpr To s_cast(From&& from)
{
    return static_cast<To>(std::forward<From>(from));
}

template<typename To, typename From>
constexpr To r_cast(From&& from)
{
    return reinterpret_cast<To>(std::forward<From>(from));
}

template<typename To, typename From>
constexpr To d_cast(From&& from)
{
    return dynamic_cast<To>(std::forward<From>(from));
}

template<typename To, typename From>
constexpr To b_cast(From&& from) noexcept
{
    return std::bit_cast<To>(std::forward<From>(from));
}

} // namespace dviglo
