// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "math.hpp"


namespace dviglo
{

constexpr u8 color_f32_to_u8(f32 value)
{
    ref_clamp(value, 0.f, 1.f);
    value = value * 255.f + 0.5f;
    return static_cast<u8>(value);
}

constexpr u32 color_f32_to_u32(f32 r, f32 g, f32 b, f32 a)
{
    u32 ret_r = color_f32_to_u8(r);
    u32 ret_g = color_f32_to_u8(g);
    u32 ret_b = color_f32_to_u8(b);
    u32 ret_a = color_f32_to_u8(a);

    return ret_r | (ret_g << 8) | (ret_b << 16) | (ret_a << 24);
}

} // namespace dviglo
