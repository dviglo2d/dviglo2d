// Copyright (c) the Dviglo project
// License: MIT

#include "timer.hpp"

#include <chrono>
#include <thread>

using namespace std;

namespace dviglo
{

static const auto start_time = chrono::steady_clock::now();

i64 get_ticks_ms()
{
    auto duration = chrono::steady_clock::now() - start_time;
    return (i64)duration_cast<chrono::milliseconds>(duration).count();
}

i64 get_ticks_ns()
{
    auto duration = chrono::steady_clock::now() - start_time;
    return (i64)duration_cast<chrono::nanoseconds>(duration).count();
}

void delay_ms(i64 ms)
{
    auto duration = chrono::milliseconds(ms);
    this_thread::sleep_for(duration);
}

void delay_ns(i64 ns)
{
    auto duration = chrono::nanoseconds(ns);
    this_thread::sleep_for(duration);
}

} // namespace dviglo
