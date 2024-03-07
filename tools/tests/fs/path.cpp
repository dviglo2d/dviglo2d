// Copyright (c) the Dviglo project
// License: MIT

#include "../force_assert.hpp"

#include <dviglo/fs/path.hpp>

using namespace dviglo;
using namespace std;


void test_io_path()
{
    {
        const StrUtf8 str("c:\\привет\\hello");
        assert(to_internal(str) == "c:/привет/hello");
    }

    {
        const StrUtf8 str("c:/привет/hello/");
        assert(trim_end_slash(str) == "c:/привет/hello");
    }

    {
        const StrUtf8 str("c:/привет/hello/");
        assert(get_parent(str) == "c:/привет/");
    }

#ifdef _WIN32
    {
        const StrUtf8 str("c:/привет/hello/");
        assert(to_win_native(str) == L"c:\\привет\\hello\\");
    }
#endif
}
