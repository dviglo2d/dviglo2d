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

    {
        const StrUtf8 path("c:/привет/hello/файл.txt");
        StrUtf8 dir_path, file_name, ext;

        split_path(path, &dir_path, &file_name, &ext);
        assert(dir_path == "c:/привет/hello/");
        assert(file_name == "файл");
        assert(ext == "txt");

        split_path(path, &dir_path, &file_name);
        assert(dir_path == "c:/привет/hello/");
        assert(file_name == "файл.txt");

        split_path(path, nullptr, &file_name);
        assert(file_name == "файл.txt");

        split_path(path, nullptr, nullptr, &ext);
        assert(ext == "txt");
    }

    {
        const StrUtf8 path("c:/привет/hello/");
        StrUtf8 dir_path, file_name, ext;

        split_path(path, &dir_path, &file_name, &ext);
        assert(dir_path == "c:/привет/hello/");
        assert(file_name == "");
        assert(ext == "");

        split_path(path, &dir_path, &file_name);
        assert(dir_path == "c:/привет/hello/");
        assert(file_name == "");

        split_path(path, nullptr, &file_name);
        assert(file_name == "");

        split_path(path, nullptr, nullptr, &ext);
        assert(ext == "");
    }
}
