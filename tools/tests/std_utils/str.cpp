// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../force_assert.h"

#include <dviglo/std_utils/str.h>

using namespace dviglo;
using namespace std;


void test_std_utils_str()
{
    {
        string str("hello");
        assert(contains(str, 'e'));
        assert(contains(str, "ell"));
        assert(contains(str, string("ell")));
    }

    {
        const string str("привет привет");
        assert(replace_all(str, "иве", "ави") == "правит правит");
    }
}
