// Copyright (c) the Dviglo project
// License: MIT

// В SDL промежутки времени u64, а должны быть i64

#pragma once

#include "../common/primitive_types.hpp"


namespace dviglo
{

inline constexpr i64 ms_per_s  = 1000;       // Миллисекунд в секунде
inline constexpr i64 us_per_s  = 1000000;    // Микросекунд в секунде
inline constexpr i64 ns_per_s  = 1000000000; // Наносекунд в секунде
inline constexpr i64 ns_per_ms = 1000000;    // Наносекунд в миллисекунде
constexpr i64 ns_per_us = 1000;       // Наносекунд в микросекунде

inline constexpr i64 s_to_ns(i64 s)  { return s * ns_per_s; }
inline constexpr i64 ns_to_s(i64 ns) { return ns / ns_per_s; }

inline constexpr i64 ms_to_ns(i64 ms) { return ms * ns_per_ms; }
inline constexpr i64 ns_to_ms(i64 ns) { return ns / ns_per_ms; }

inline constexpr i64 us_to_ns(i64 us) { return us * ns_per_us; }
inline constexpr i64 ns_to_us(i64 ns) { return ns / ns_per_us; }

i64 get_ticks_ms();
i64 get_ticks_ns();

void delay_ms(i64 ms);
void delay_ns(i64 ns);

} // namespace dviglo
