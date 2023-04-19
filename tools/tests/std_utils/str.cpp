// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../force_assert.h"

#include <dviglo/std_utils/str.h>

using namespace dviglo;
using namespace std;


void test_std_utils_str()
{
    {
        const string str("hello");
        assert(contains(str, 'e'));
        assert(contains(str, "ell"));
        assert(contains(str, string("ell")));
    }

    {
        static_assert(to_lower('G') == 'g');
        static_assert(to_upper('d') == 'D');
    }

    {
        const string str("привет привет");
        assert(replace_all(str, "иве", "ави") == "правит правит");
    }

    {
        const string str = "heLlo";
        assert(replace_all(str, 'L', '2') == "he2lo");
        assert(replace_all(str, 'l', '2', false) == "he22o");
    }

    {
        const string str = "привет";
        size_t offset = 0;
        assert(next_code_point(str, offset) == U'\x43f');
        assert(next_code_point(str, offset) == U'\x440');
        assert(next_code_point(str, offset) == U'\x438');
        assert(next_code_point(str, offset) == U'\x432');
        assert(next_code_point(str, offset) == U'\x435');
        assert(next_code_point(str, offset) == U'\x442');
    }
}
