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
    }

    {
        const string str("привет🍌");
        assert(contains(str, "🍌"));
        assert(contains(str, string("иве")));
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
        const string str = "привет🍌";
        size_t offset = 0;
        // https://en.cppreference.com/w/cpp/language/character_literal
        assert(next_code_point(str, offset) == U'п');
        assert(next_code_point(str, offset) == U'р');
        assert(next_code_point(str, offset) == U'и');
        assert(next_code_point(str, offset) == U'в');
        assert(next_code_point(str, offset) == U'е');
        assert(next_code_point(str, offset) == U'т');
        assert(next_code_point(str, offset) == U'🍌');
    }
}
