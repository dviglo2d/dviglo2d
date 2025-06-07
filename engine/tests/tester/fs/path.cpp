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
    // 3) В Windows регистр символов не имеет значения, но библиотека при сравнении путей это не учитывает
    {
        fs::path m(R"(/привет\пока:/)");
        auto aa = m.root_name();
        auto bb = m.root_directory();
        auto cc = m.root_path();
        auto dd = fs::path("//\\usr\\///\\\\////lib/\\\\").lexically_normal().string();
        auto ff = m.string();

        cout << fs::path("/привет/").root_directory().string() << endl;
        cout << fs::path(R"(//привет/\пока\/)").string() << endl;
        cout << fs::path(R"(/привет\пока:/)").string() << endl;
        cout << fs::path(R"(\привет/)").root_directory().string() << endl;



        
        // Такое сравнение не работает даже в Windows
        assert(fs::path("Hello") != fs::path("hello"));

        // Библиотека хранить путь как есть (только кодировку меняет на нативную)
        assert(fs::path(R"(/привет\пока:/)").string() == R"(/привет\пока:/)");
        assert(fs::path(R"(//привет/\пока\/)").string() == R"(//привет/\пока\/)");
        assert(fs::path(R"(\\привет*?\//пока\/)").string() == R"(\\привет*?\//пока\/)");
        assert(fs::path(R"(:\\//привет\//пока\/)").string() == R"(:\\//привет\//пока\/)");

        // Начальный бэкслэш интерпретируется по разному в разных ОС
#if DV_WINDOWS
        // Обратный слэш - это разделитель и имя корня
        assert(fs::path(R"(\)").root_name().string() == "");
        assert(fs::path(R"(\)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\)").root_path().string() == R"(\)");
        assert(fs::path(R"(\)").filename().string() == "");

        assert(fs::path(R"(\привет)").root_name().string() == "");
        assert(fs::path(R"(\привет)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\привет)").root_path().string() == R"(\)");
        assert(fs::path(R"(\привет)").filename().string() == "привет");
#elif DV_LINUX
        // Обратный слэш - это часть имени файла
        assert(fs::path(R"(\)").root_name().string() == "");
        assert(fs::path(R"(\)").root_directory().string() == "");
        assert(fs::path(R"(\)").root_path().string() == "");
        assert(fs::path(R"(\)").filename().string() == R"(\)");

        assert(fs::path(R"(\привет)").root_name().string() == "");
        assert(fs::path(R"(\привет)").root_directory().string() == "");
        assert(fs::path(R"(\привет)").root_path().string() == "");
        assert(fs::path(R"(\привет)").filename().string() == R"(\привет)");
#else
        #error
#endif

        // Начальный слэш - это имя корня во всех ОС
        assert(fs::path("/привет").root_name().string() == "");
        assert(fs::path("/привет").root_directory().string() == "/");
        assert(fs::path("/привет").root_path().string() == "/");
        assert(fs::path("/привет").filename().string() == "привет");
        assert(fs::path("/привет/").filename().string() == "");

        // Путь, состоящий из одного слэша, имеет особенность в MinGW
#if DV_WINDOWS_MSVC || DV_LINUX_GCC || DV_LINUX_CLANG
        assert(fs::path("/").root_name().string() == "");
        assert(fs::path("/").root_directory().string() == "/");
        assert(fs::path("/").root_path().string() == "/");
#elif DV_WINDOWS_MINGW
        // Тут MinGW противоречит предыдущим тестам и меняет слэш на бэкслэш.
        // Значит нужно всегда использовать generic_string()
        assert(fs::path("/").root_name().string() == "");
        assert(fs::path("/").root_directory().string() == R"(\)");
        assert(fs::path("/").root_path().string() == R"(\)");
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
        // Идущие подряд слэши и бэкслеши интерпретируются как один разделитель
        assert(fs::path(R"(/usr\///\////lib)").lexically_normal().string() == R"(\usr\lib)");
        assert(fs::path(R"(//\usr\///\\////lib/\\)").lexically_normal().string() == R"(\usr\lib\)");
        assert(fs::path(R"(\\\usr\///\\////lib/\\)").lexically_normal().string() == R"(\usr\lib\)");
#elif DV_LINUX
        // Обратный слэш - это часть имени файла
        assert(fs::path(R"(/usr\\///\////lib)").lexically_normal().string() == R"(/usr\\/\/lib)");
        assert(fs::path(R"(//\usr\///\\////lib/\\)").lexically_normal().string() == R"(/\usr\/\\/lib/\\)");
        assert(fs::path(R"(\\\usr\///\\////lib)").lexically_normal().string() == R"(\\\usr\/\\/lib)");
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
