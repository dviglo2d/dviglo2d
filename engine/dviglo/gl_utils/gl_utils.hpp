// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../math/rect.hpp"


namespace dviglo
{

inline u8 get_r(u32 rgba)
{
    return u8(rgba & 0x000000FF);
}

inline u8 get_g(u32 rgba)
{
    return u8((rgba & 0x0000FF00) >> 8);
}

inline u8 get_b(u32 rgba)
{
    return u8((rgba & 0x00FF0000) >> 16);
}

inline u8 get_a(u32 rgba)
{
    return u8((rgba & 0xFF000000) >> 24);
}

inline u32 to_rgba(u8 r, u8 g, u8 b, u8 a)
{
    u32 u32_a = u32(a) << 24;
    u32 u32_b = u32(b) << 16;
    u32 u32_g = u32(g) << 8;
    u32 u32_r = u32(r);

    return u32_a | u32_b | u32_g | u32_r;
}

// Начало координат в левом нижнем углу
IntRect get_viewport();

}  // namespace dviglo
