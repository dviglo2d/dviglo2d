// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../force_assert.h"

#include <dviglo/io/path.h>

using namespace dviglo;
using namespace std;


void test_io_path()
{
    {
        const string str("c:\\привет\\hello");
        assert(to_internal(str) == "c:/привет/hello");
    }

    {
        const string str("c:/привет/hello/");
        assert(trim_end_slash(str) == "c:/привет/hello");
    }

    {
        const string str("c:/привет/hello/");
        assert(get_parent(str) == "c:/привет/");
    }

#ifdef _WIN32
    {
        const string str("c:/привет/hello/");
        assert(to_win_native(str) == L"c:\\привет\\hello\\");
    }
#endif
}
