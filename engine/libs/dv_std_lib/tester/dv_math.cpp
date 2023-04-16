// Copyright (c) the Dviglo project
// License: MIT

#include <dv_force_assert.hpp>

#include <dv_math.hpp>

using namespace dviglo;


static constexpr bool test_ref_clamp()
{
    i32 val = 1;
    ref_clamp(val, 2, 7);
    return val == 2;
}
static_assert(test_ref_clamp());

void test_math()
{
    {
        assert(round_to_i32(1.15f) == 1);
        assert(round_to_i32(1.5f) == 2);
    }
}
