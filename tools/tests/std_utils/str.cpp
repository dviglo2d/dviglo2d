// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../force_assert.h"

#include <dviglo/std_utils/str.h>

using namespace dviglo;
using namespace std;


void test_std_utils_str()
{
    {
        string str("привет");
        assert(contains(str, 'и'));
        assert(contains(str, "иве"));
        assert(contains(str, string("иве")));
    }
}
