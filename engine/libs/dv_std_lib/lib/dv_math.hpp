// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_platform.hpp"
#include "dv_primitive_types.hpp"

#include <cmath>
#include <type_traits>


namespace dviglo
{

// Угол в радианах
inline void sin_cos(const f32 angle, f32& out_sin, f32& out_cos)
{
#if DV_WINDOWS_MSVC
    out_sin = sinf(angle);
    out_cos = cosf(angle);
#else
    sincosf(angle, &out_sin, &out_cos);
#endif
}

// Угол в радианах
inline void sin_cos(const f64 angle, f64& out_sin, f64& out_cos)
{
#if DV_WINDOWS_MSVC
    out_sin = sin(angle);
    out_cos = cos(angle);
#else
    sincos(angle, &out_sin, &out_cos);
#endif
}

// Типы, которые быстрее передавать по значению (с размером 16 байт и меньше)
template <typename T>
concept pass_by_val = std::is_same_v<T, i32> || std::is_same_v<T, f32>;

// Типы, которые быстрее передавать по ссылке
template <typename T>
concept pass_by_ref = !pass_by_val<T>;

// Меняет переменную, а не возвращает значение
template <pass_by_val T>
constexpr void ref_clamp(T& inout_val, const T min_val, const T max_val)
{
    if (inout_val < min_val)
        inout_val = min_val;
    else if (inout_val > max_val)
        inout_val = max_val;
}

// Меняет переменную, а не возвращает значение
template <pass_by_ref T>
constexpr void ref_clamp(T& inout_val, const T& min_val, const T& max_val)
{
    if (inout_val < min_val)
        inout_val = min_val;
    else if (inout_val > max_val)
        inout_val = max_val;
}

// Меняет переменную, а не возвращает значение
template <pass_by_val T>
constexpr void ref_min(T& inout_val, const T other_val)
{
    if (inout_val > other_val)
        inout_val = other_val;
}

// Меняет переменную, а не возвращает значение
template <pass_by_val T>
constexpr void ref_max(T& inout_val, const T other_val)
{
    if (inout_val < other_val)
        inout_val = other_val;
}

// Округляет до ближайшего целого
template <typename T>
i32 round_to_i32(T val) { return static_cast<i32>(std::round(val)); }

} // namespace dviglo
