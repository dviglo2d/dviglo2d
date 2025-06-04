// Copyright (c) the Dviglo project
// License: MIT

#include "../force_assert.hpp"

#include <dviglo/fs/path.hpp>

using namespace dviglo;
using namespace std;

namespace fs = std::filesystem;


void test_fs_path()
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

#ifdef DV_WINDOWS
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

    // Проверяем, что стандартная библиотека работает предсказуемым образом
    {
        string m = "/usr///\\\\////lib";
        auto aa = fs::path(m).root_name();
        auto bb = fs::path(m).root_directory();
        auto cc = fs::path(m).root_path();
        auto dd = fs::path("/usr\\///\\\\////lib").lexically_normal().string();

        assert(fs::path("").root_name() == "");
        assert(fs::path("").root_directory() == "");
        assert(fs::path("").root_path() == "");

        assert(fs::path("z:\\").root_name() == "z:");
        assert(fs::path("z:\\").root_directory() == "\\");
        assert(fs::path("z:\\").root_path() == "z:\\");

        assert(fs::path("z:/").root_name() == "z:");
        assert(fs::path("z:/").root_directory() == "/");
        assert(fs::path("z:/").root_path() == "z:/");

        assert(fs::path("z:").root_name() == "z:");
        assert(fs::path("z:").root_directory() == "");
        assert(fs::path("z:").root_path() == "z:");

        assert(fs::path("/").root_name() == "");
        assert(fs::path("/").root_directory() == "/");
        assert(fs::path("/").root_path() == "/");

        assert(fs::path("\\").root_name() == "");
        assert(fs::path("\\").root_directory() == "\\");
        assert(fs::path("\\").root_path() == "\\");

        assert(fs::path("/привет").root_name() == "");
        assert(fs::path("/привет").root_directory() == "/");
        assert(fs::path("/привет").root_path() == "/");

        assert(fs::path("//сервер").root_name() == "//сервер");
        assert(fs::path("//сервер").root_directory() == "");
        assert(fs::path("//сервер").root_path() == "//сервер");

        assert(fs::path("///привет").root_name() == "");
        assert(fs::path("///привет").root_directory() == "///");
        assert(fs::path("///привет").root_path() == "///");

        assert(fs::path("/usr///\\\\////lib") == "/usr///lib");

#if DV_WINDOWS
        assert(fs::path("//\\usr\\///\\\\////lib").lexically_normal().string() == "\\usr\\lib");
#elif DV_LINUX
        assert(fs::path("//\\usr\\///\\\\////lib").lexically_normal().string() == "/usr/lib");
#else
    #error
#endif

        //assert(fs::path("/привет\\hello/").root_name() == "");
        assert(fs::path("/привет\\hello/").root_directory() == "/");
        assert(fs::path("\\привет\\hello/").root_directory() == "\\");
        assert(fs::path("/привет\\hello/").root_name() == "");

        assert(fs::path("//привет\\hello/").root_directory() == "/");
       //assert(fs::path("//привет\\hello/").root_name() == "/");



        assert(fs::path("z:/привет\\hello/").root_name() == "z:");
        assert(fs::path("z:/привет\\hello/").root_directory() == "/");
        assert(fs::path("z:\\привет\\hello/").root_directory() == "\\");
        assert(fs::path("z://привет\\hello/").root_directory() == "/");
    }

    {
       /* assert(trim_end_slashes("").string() == "");
        assert(trim_end_slashes("/").string() == "");
        assert(trim_end_slashes("\\/").string() == "");
        assert(trim_end_slashes("z:\\").string() == "z:");
        assert(trim_end_slashes("z://").string() == "z:");
        assert(trim_end_slashes("c:\\привет/hello/").string() == "c:\\привет/hello");
        assert(trim_end_slashes("./привет\\hello/").string() == "./привет\\hello");
        assert(trim_end_slashes("c:/привет\\hello/\\/").generic_string() == "c:/привет/hello");*/
    }
}
