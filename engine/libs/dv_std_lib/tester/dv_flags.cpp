// Copyright (c) the Dviglo project
// License: MIT

#include <dv_flags.hpp>
#include <dv_primitive_types.hpp>

using namespace dviglo;


enum class TestEnum : u32
{
    e0 = 0,
    e1 = 1 << 1,
    e2 = 1 << 2,
    e3 = 1 << 3,
    not_e2 = ~e2,
    e1_e2 = e1 | e2,
};
DV_FLAGS(TestEnum);

static_assert(!TestEnum::e0 == true);
static_assert(!!TestEnum::e2 == true);

static_assert(~TestEnum::e2 == TestEnum::not_e2);

static_assert(TestEnum::e3 == 1 << 3);
static_assert(1 << 3 == TestEnum::e3);
static_assert(TestEnum::e3 != 1 << 2);
static_assert(1 << 2 != TestEnum::e3);

static_assert((TestEnum::e1_e2 & TestEnum::e1) == TestEnum::e1);
static_assert((TestEnum::e1_e2 ^ TestEnum::e1_e2) == TestEnum::e0);
static_assert((TestEnum::e1 | TestEnum::e2) == TestEnum::e1_e2);

static constexpr bool test_bit_and_assign()
{
    TestEnum val = TestEnum::e1_e2;
    val &= TestEnum::e1;
    return val == TestEnum::e1;
}
static_assert(test_bit_and_assign());

static constexpr bool test_bit_xor_assign()
{
    TestEnum val = TestEnum::e1_e2;
    val ^= TestEnum::e1_e2;
    return val == TestEnum::e0;
}
static_assert(test_bit_xor_assign());

static constexpr bool test_bit_or_assign()
{
    TestEnum val = TestEnum::e1;
    val |= TestEnum::e2;
    return val == TestEnum::e1_e2;
}
static_assert(test_bit_or_assign());

void test_flags()
{
}
