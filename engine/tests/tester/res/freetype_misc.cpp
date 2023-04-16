#include <dviglo/res/freetype_misc.hpp>

#include <limits>

using namespace dviglo;
using namespace std;


static_assert( 1.f / 64.f == 0.015625f);
static_assert(31.f / 64.f == 0.484375f);
static_assert(32.f / 64.f == 0.5f);
static_assert(33.f / 64.f == 0.515625f);
static_assert(63.f / 64.f == 0.984375f);

static_assert(round_to_pixels( 2 * 64 + 31) ==  2); //  2.484375 ->  2
static_assert(round_to_pixels( 2 * 64 + 32) ==  3); //  2.5      ->  3
static_assert(round_to_pixels(-2 * 64 - 32) == -2); // -2.5      -> -2 // Отличается от round()
static_assert(round_to_pixels(-2 * 64 - 33) == -3); // -2.515625 -> -3

static_assert(numeric_limits<i32>::max() == 33554431 * 64 + 63);
static_assert(round_to_pixels(33554431 * 64 + 63) == 33554432); // 33554431.984375 -> 33554432

static_assert(numeric_limits<i32>::min() == -33554432 * 64 - 0);
static_assert(round_to_pixels(-33554432 * 64 - 0) == -33554432); // -33554432.0 -> -33554432

static_assert(numeric_limits<i32>::min() + 1 == -33554431 * 64 - 63);
static_assert(round_to_pixels(-33554431 * 64 - 63) == -33554432); // -33554431.984375 -> -33554432

void test_res_freetype_misc()
{
}
