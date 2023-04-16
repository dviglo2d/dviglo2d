// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../common/primitive_types.hpp"

#include <freetype/internal/ftobjs.h> // FT_PIX_FLOOR, FT_PIX_ROUND


namespace dviglo
{

// Для чисел с фиксированной точкой в формате 26.6
constexpr i32 round_to_pixels(FT_Pos value)
{
#if false
    // Может вызвать переполнение, так как сперва прибавляет 32
    return static_cast<i32>(FT_PIX_ROUND(value) >> 6);
#else
    i32 ret = static_cast<i32>(FT_PIX_FLOOR(value) >> 6);

    FT_Pos frac_part = value & 63; // value & 0b111111
    if (frac_part >= 32) // Отрицательные числа в дополненном коде
        ++ret;

    return ret;
#endif
}

} // namespace dviglo
