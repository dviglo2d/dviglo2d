// Copyright (c) the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#pragma once

#include "../common/primitive_types.hpp"

#include <type_traits>


namespace dviglo
{

void sin_cos(f32 angle, f32& sin, f32& cos);

/// Типы, которые быстрее передавать по значению
template <typename T>
concept pass_by_val = std::is_same_v<T, i32> || std::is_same_v<T, f32>;

template <typename T>
concept pass_by_ref = !pass_by_val<T>;

/// Ограничивает переменную на месте, а не возвращает значение
template<pass_by_val T>
void ref_clamp(T& val, const T min_val, const T max_val)
{
    if (val < min_val)
        val = min_val;
    else if (val > max_val)
        val = max_val;
}

/// Ограничивает переменную на месте, а не возвращает значение
template<pass_by_ref T>
void ref_clamp(T& val, const T& min_val, const T& max_val)
{
    if (val < min_val)
        val = min_val;
    else if (val > max_val)
        val = max_val;
}

} // namespace dviglo
