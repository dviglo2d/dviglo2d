// Copyright (c) the Dviglo project
// License: MIT

#include "../force_assert.hpp"

#include <dviglo/fs/path.hpp>

#include <iostream>

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

    // Проверяем, что стандартная библиотека работает предсказуемым образом.
    // Отличия Linux и Windows:
    // 1) Linux: В именах файлов (или папок) разрешены любые символы, кроме / и '\0'.
    //           \, z: - обычные имена файлов
    //    Windows: \ и / - разделители, некоторые символы запрещены: https://ru.wikipedia.org/wiki/Имя_файла
    // 2) Linux: root_name (имя корня) всегда пустое
    //    Windows: \\server_name или z: - имена корней
    // 3) В Windows регистр символов не имеет значения, но библиотека это не учитывает
    {
        fs::path m("\\привет\\hello/");
        auto aa = m.root_name();
        auto bb = m.root_directory();
        auto cc = m.root_path();
        auto dd = fs::path("//\\usr\\///\\\\////lib/\\\\").lexically_normal().string();

        cout << fs::path("/привет/").root_directory().string() << endl;
        cout << fs::path(R"(//привет/\пока\/)").string() << endl;
        cout << fs::path(R"(\//привет\//пока\/)").string() << endl;

        // Такое сравнение не работает даже в Windows
        assert(fs::path("Hello") != fs::path("hello"));

#if DV_WINDOWS
        // Обратный слэш - это разделитель
        assert(fs::path(R"(\)").root_name() == "");
        assert(fs::path(R"(\)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\)").root_path().string() == R"(\)");
        assert(fs::path(R"(\)").filename() == "");
#elif DV_LINUX
        // Обратный слэш - это имя файла
        assert(fs::path(R"(\)").root_name() == "");
        assert(fs::path(R"(\)").root_directory() == "");
        assert(fs::path(R"(\)").root_path() == "");
        assert(fs::path(R"(\)").filename().string() == R"(\)");
#else
        #error
#endif

#if DV_WINDOWS
        assert(fs::path("z:\\").root_name() == "z:");
        assert(fs::path("z:\\").root_directory() == "\\");
        assert(fs::path("z:\\").root_path() == "z:\\");

        assert(fs::path("z:/").root_name() == "z:");
        assert(fs::path("z:/").root_directory() == "/");
        assert(fs::path("z:/").root_path() == "z:/");

        assert(fs::path("z:").root_name() == "z:");
        assert(fs::path("z:").root_directory() == "");
        assert(fs::path("z:").root_path() == "z:");
#elif DV_LINUX
        assert(fs::path("z:\\").root_name() == "");
        assert(fs::path("z:\\").root_directory() == "");
        assert(fs::path("z:\\").root_path() == "");

        assert(fs::path("z:/").root_name() == "");
        assert(fs::path("z:/").root_directory() == "");
        assert(fs::path("z:/").root_path() == "");

        assert(fs::path("z:").root_name() == "");
        assert(fs::path("z:").root_directory() == "");
        assert(fs::path("z:").root_path() == "");
#else
    #error
#endif

#if DV_WINDOWS
        // MSVC и MinGW не меняют разделители в пути
        assert(fs::path(R"(/привет\пока/)").string() == R"(/привет\пока/)");
        assert(fs::path(R"(//привет/\пока\/)").string() == R"(//привет/\пока\/)");
        assert(fs::path(R"(\\привет\//пока\/)").string() == R"(\\привет\//пока\/)");
        assert(fs::path(R"(\\//привет\//пока\/)").string() == R"(\\//привет\//пока\/)");
#elif DV_LINUX
        assert(fs::path(R"(/привет\пока/)").string() == R"(/привет/пока/)");
        assert(fs::path(R"(//привет/\пока\/)").string() == R"(//привет//пока//)");
        assert(fs::path(R"(\\привет\//пока\/)").string() == R"(//привет///пока//)");
        assert(fs::path(R"(\\//привет\//пока\/)").string() == R"(////привет///пока//)");
#else
        #error
#endif

        assert(fs::path("/привет").root_name() == "");
        assert(fs::path("/привет").root_directory().string() == "/");
        assert(fs::path("/привет").root_path().string() == "/");
        assert(fs::path("/привет").filename().string() == "привет");
        assert(fs::path("/привет/").filename().string() == "");

#if DV_WINDOWS
        // MSVC и MinGW не меняют корневую папку
        assert(fs::path(R"(\привет/)").root_name() == "");
        assert(fs::path(R"(\привет/)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\привет/)").root_path().string() == R"(\)");
#elif DV_LINUX
        assert(fs::path(R"(\привет/)").root_name() == "");
        assert(fs::path(R"(\привет/)").root_directory().string() == R"(/)");
        assert(fs::path(R"(\привет/)").root_path().string() == R"(/)");
#else
        #error
#endif

#if DV_WINDOWS
        // MSVC и MinGW не меняют корневую папку
        assert(fs::path(R"(\)").root_name() == "");
        assert(fs::path(R"(\)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\)").root_path().string() == R"(\)");
#elif DV_LINUX
        assert(fs::path(R"(\)").root_name() == "");
        assert(fs::path(R"(\)").root_directory().string() == "/");
        assert(fs::path(R"(\)").root_path().string() == "/");
#else
        #error
#endif

#if DV_WINDOWS_MSVC || DV_LINUX_GCC || DV_LINUX_CLANG
        assert(fs::path("/").root_name() == "");
        assert(fs::path("/").root_directory().string() == "/");
        assert(fs::path("/").root_path().string() == "/");
#elif DV_WINDOWS_MINGW
        // Тут MinGW противоречит предыдущим тестам и меняет корневую папку, если путь состоит только из /.
        // Значит нужно всегда использовать generic_string()
        assert(fs::path("/").root_name() == "");
        assert(fs::path("/").root_directory().string() == R"(\)");
        assert(fs::path("/").root_path().string() == R"(\)");
#else
        #error
#endif

        assert(fs::path("/привет/").root_name() == "");
        assert(fs::path("/привет/").root_directory().string() == "/");
        assert(fs::path("/привет/").root_path().string() == "/");
        assert(fs::path("/привет/").filename().string() == "");



        assert(fs::path("/привет").root_path().string() == "/");
        assert(fs::path("/привет").filename().string() == "привет");





        assert(fs::path("/").root_name() == "");
        assert(fs::path("/").root_directory() == "/");
        assert(fs::path("/").root_path() == "/");

#if DV_WINDOWS
        assert(fs::path("\\").root_name() == "");
        assert(fs::path("\\").root_directory() == "\\");
        assert(fs::path("\\").root_path() == "\\");
#elif DV_LINUX
        assert(fs::path("\\").root_name() == "");
        assert(fs::path("\\").root_directory() == "");
        assert(fs::path("\\").root_path() == "");
#else
        #error
#endif

        assert(fs::path("/привет").root_name() == "");
        assert(fs::path("/привет").root_directory() == "/");
        assert(fs::path("/привет").root_path() == "/");

        // \\server\share\file
#if DV_WINDOWS_MSVC
        assert(fs::path("//сервер").root_name() == "//сервер");
        assert(fs::path("//сервер").root_directory() == "");
        assert(fs::path("//сервер").root_path() == "//сервер");
#elif DV_LINUX_CLANG || DV_LINUX_GCC || DV_WINDOWS_MINGW
        assert(fs::path("//сервер").root_name() == "");
        assert(fs::path("//сервер").root_directory() == "/");
        assert(fs::path("//сервер").root_path() == "/");
#else
        #error
#endif
        assert(fs::path("///привет").root_name() == "");
        assert(fs::path("///привет").root_directory() == "/");
        assert(fs::path("///привет").root_path() == "/");

#if DV_WINDOWS
        assert(fs::path("/usr\\///\\\\////lib") == "/usr///lib");
        assert(fs::path("//\\usr\\///\\\\////lib/\\\\").lexically_normal().string() == "\\usr\\lib\\");
#elif DV_LINUX
        assert(fs::path("/usr\\///\\\\////lib") == "/usr\\/\\\\/lib");
        assert(fs::path("//\\usr\\///\\\\////lib/\\\\").lexically_normal().string() == "/\\usr\\/\\\\/lib/\\\\");
#else
        #error
#endif

        //assert(fs::path("/привет\\hello/").root_name() == "");
        assert(fs::path("/привет\\hello/").root_directory() == "/");

#if DV_WINDOWS
        assert(fs::path("\\привет\\hello/").root_directory() == "\\");
#elif DV_LINUX
        assert(fs::path("\\привет\\hello/").root_directory() == "");
#else
        #error
#endif

        assert(fs::path("/привет\\hello/").root_name() == "");

        assert(fs::path("//привет\\hello/").root_directory() == "/");
       //assert(fs::path("//привет\\hello/").root_name() == "/");
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
