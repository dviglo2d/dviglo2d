// Copyright (c) the Dviglo project
// License: MIT

#include <dv_force_assert.hpp>

#include <dv_fs.hpp>
#include <dv_platform.hpp>

using namespace dviglo;


void test_fs()
{
    // Проверяем, что стандартная библиотека работает предсказуемым образом.
    // Отличия путей в Linux и в Windows:
    // 1) Linux: В именах файлов (папок) разрешены любые символы, кроме / и '\0'
    //    Windows: \ и / - разделители, некоторые символы запрещены: https://ru.wikipedia.org/wiki/Имя_файла
    // 2) Linux: root_name (имя корня) всегда пустое
    //    Windows: \\server_name или z: - имена корней
    // 3) В Windows регистр символов не имеет значения, но библиотека при сравнении путей это не учитывает
    {
        // Библиотека всегда хранить путь как есть (только кодировку меняет на нативную)
        assert(fs::path(R"(/привет\пока:/)").string() == R"(/привет\пока:/)");
        assert(fs::path(R"(//привет/\пока\/)").string() == R"(//привет/\пока\/)");
        assert(fs::path(R"(\\привет*?\//пока\/)").string() == R"(\\привет*?\//пока\/)");
        assert(fs::path(R"(:\\//привет\//пока\/)").string() == R"(:\\//привет\//пока\/)");

        // Регистр имеет значение даже в Windows
        assert(fs::path("Hello") != fs::path("hello"));

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

        // При объединении путей в разных ОС используется разный разделитель
#if DV_WINDOWS
        assert((fs::path("привет") / "пока").string() == R"(привет\пока)");
#elif DV_LINUX
        assert((fs::path("привет") / "пока").string() == "привет/пока");
#else
        #error
#endif

        // Новый разделитель не добавляется
        assert((fs::path("привет/") / "пока").string() == "привет/пока");
        assert((fs::path("привет//") / "пока").string() == "привет//пока");
#if DV_WINDOWS
        assert((fs::path(R"(привет//\)") / "пока").string() == R"(привет//\пока)");
#elif DV_LINUX
        // Обратный слэш - это имя папки
        assert((fs::path(R"(привет//\)") / "пока").string() == R"(привет//\/пока)");
#else
        #error
#endif

        // Если второй путь начинается с корневой папки, то он заменяет путь
        // https://en.cppreference.com/w/cpp/filesystem/path/append.html
        assert((fs::path("привет") / "/пока").string() == "/пока");
#if DV_WINDOWS
        assert((fs::path("z:/") / "/пока").string() == "z:/пока");
        assert((fs::path("z:") / "/пока").string() == "z:/пока");
        assert((fs::path(R"(z:\)") / "/пока").string() == "z:/пока");
        assert((fs::path("z:/") / R"(\пока)").string() == R"(z:\пока)");
        assert((fs::path("z:/") / R"(d:\пока)").string() == R"(d:\пока)");
#endif

        // Начальный бэкслэш интерпретируется по разному в разных ОС
        assert(fs::path(R"(\)").root_name().string() == "");
        assert(fs::path(R"(\привет)").string() == R"(\привет)");
        assert(fs::path(R"(\)").string() == R"(\)");
        assert(fs::path(R"(\привет)").root_name().string() == "");
#if DV_WINDOWS
        // Обратный слэш - это разделитель и корневая папка
        assert(fs::path(R"(\)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\)").root_path().string() == R"(\)");
        assert(fs::path(R"(\)").filename().string() == "");

        assert(fs::path(R"(\привет)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\привет)").root_path().string() == R"(\)");
        assert(fs::path(R"(\привет)").filename().string() == "привет");
#elif DV_LINUX
        // Обратный слэш - это часть имени файла
        assert(fs::path(R"(\)").root_directory().string() == "");
        assert(fs::path(R"(\)").root_path().string() == "");
        assert(fs::path(R"(\)").filename().string() == R"(\)");

        assert(fs::path(R"(\привет)").root_directory().string() == "");
        assert(fs::path(R"(\привет)").root_path().string() == "");
        assert(fs::path(R"(\привет)").filename().string() == R"(\привет)");
#else
        #error
#endif

        // Начальный слэш - это корневая папка во всех ОС
        assert(fs::path("/привет").root_name().string() == "");
        assert(fs::path("/привет").root_directory().string() == "/");
        assert(fs::path("/привет").root_path().string() == "/");
        assert(fs::path("/привет").filename().string() == "привет");
        assert(fs::path("/привет/").filename().string() == "");

        // Путь, состоящий из одного слэша, имеет особенность в MinGW
        assert(fs::path("/").string() == "/");
        assert(fs::path("/").root_name().string() == "");
#if DV_WINDOWS_MSVC || DV_LINUX_GCC || DV_LINUX_CLANG
        assert(fs::path("/").root_directory().string() == "/");
        assert(fs::path("/").root_path().string() == "/");
#elif DV_WINDOWS_MINGW
        // Тут MinGW противоречит предыдущим тестам и меняет слэш на бэкслэш.
        // Значит нужно всегда использовать generic_string()
        assert(fs::path("/").root_directory().string() == R"(\)");
        assert(fs::path("/").root_path().string() == R"(\)");
#else
        #error
#endif

        // В Linux имя корня всегда пустое
#if DV_WINDOWS
        assert(fs::path(R"(z:\)").root_name().string() == "z:");
        assert(fs::path(R"(z:\)").root_directory().string() == R"(\)");
        assert(fs::path(R"(z:\)").root_path().string() == R"(z:\)");
        assert(fs::path(R"(z:\)").filename().string() == "");

        assert(fs::path("z:/").root_name().string() == "z:");
        assert(fs::path("z:/").root_directory().string() == "/");
        assert(fs::path("z:/").root_path().string() == "z:/");
        assert(fs::path("z:/").filename().string() == "");

        assert(fs::path("z:").root_name().string() == "z:");
        assert(fs::path("z:").root_directory().string() == "");
        assert(fs::path("z:").root_path().string() == "z:");
        assert(fs::path("z:").filename().string() == "");
#elif DV_LINUX
        // z: - это имя файла (папки)
        assert(fs::path(R"(z:\)").root_name().string() == "");
        assert(fs::path(R"(z:\)").root_directory().string() == "");
        assert(fs::path(R"(z:\)").root_path().string() == "");
        assert(fs::path(R"(z:\)").filename().string() == R"(z:\)");

        assert(fs::path("z:/").root_name().string() == "");
        assert(fs::path("z:/").root_directory().string() == "");
        assert(fs::path("z:/").root_path().string() == "");
        assert(fs::path("z:/").filename().string() == "");

        assert(fs::path("z:").root_name().string() == "");
        assert(fs::path("z:").root_directory().string() == "");
        assert(fs::path("z:").root_path().string() == "");
        assert(fs::path("z:").filename().string() == "z:");
#else
    #error
#endif

        // В Windows пусть к сетевому ресурсу начинается с \\, но MinGW об этом не знает
        assert(fs::path(R"(\\сервер/папка)").string() == R"(\\сервер/папка)");
        assert(fs::path("//сервер").string() == "//сервер");
        assert(fs::path(R"(\/сервер\папка)").string() == R"(\/сервер\папка)");
#if DV_WINDOWS_MSVC
        assert(fs::path(R"(\\сервер/папка)").root_name().string() == R"(\\сервер)");
        assert(fs::path(R"(\\сервер/папка)").root_directory().string() == "/");
        assert(fs::path(R"(\\сервер/папка)").root_path().string() == R"(\\сервер/)");
        assert(fs::path(R"(\\сервер/папка)").is_absolute());

        assert(fs::path("//сервер").root_name().string() == "//сервер");
        assert(fs::path("//сервер").root_directory().string() == "");
        assert(fs::path("//сервер").root_path().string() == "//сервер");
        assert(fs::path("//сервер").filename().string() == "");
        assert(fs::path("//сервер").is_absolute());

        assert(fs::path(R"(\/сервер\папка)").root_name().string() == R"(\/сервер)");
        assert(fs::path(R"(\/сервер\папка)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\/сервер\папка)").root_path().string() == R"(\/сервер\)");
        assert(fs::path(R"(\/сервер\папка)").is_absolute());
#elif DV_WINDOWS_MINGW
        assert(fs::path(R"(\\сервер/папка)").root_name().string() == "");
        assert(fs::path(R"(\\сервер/папка)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\\сервер/папка)").root_path().string() == R"(\)");
        assert(!fs::path(R"(\\сервер/папка)").is_absolute());

        assert(fs::path("//сервер").root_name().string() == "");
        assert(fs::path("//сервер").root_directory().string() == "/");
        assert(fs::path("//сервер").root_path().string() == "/");
        assert(fs::path("//сервер").filename().string() == "сервер");
        assert(!fs::path("//сервер").is_absolute());

        assert(fs::path(R"(\/сервер\папка)").root_name().string() == "");
        assert(fs::path(R"(\/сервер\папка)").root_directory().string() == R"(\)");
        assert(fs::path(R"(\/сервер\папка)").root_path().string() == R"(\)");
        assert(!fs::path(R"(\/сервер\папка)").is_absolute());
#elif DV_LINUX_CLANG || DV_LINUX_GCC
        assert(fs::path(R"(\\сервер/папка)").root_name().string() == "");
        assert(fs::path(R"(\\сервер/папка)").root_directory().string() == "");
        assert(fs::path(R"(\\сервер/папка)").root_path().string() == "");
        assert(!fs::path(R"(\\сервер/папка)").is_absolute());

        assert(fs::path("//сервер").root_name().string() == "");
        assert(fs::path("//сервер").root_directory().string() == "/");
        assert(fs::path("//сервер").root_path().string() == "/");
        assert(fs::path("//сервер").filename().string() == "сервер");
        assert(fs::path("//сервер").is_absolute());

        assert(fs::path(R"(\/сервер\папка)").root_name().string() == "");
        assert(fs::path(R"(\/сервер\папка)").root_directory().string() == "");
        assert(fs::path(R"(\/сервер\папка)").root_path().string() == "");
        assert(!fs::path(R"(\/сервер\папка)").is_absolute());
#else
        #error
#endif

        assert(fs::path("///привет").root_name() == "");
        assert(fs::path("///привет").root_directory() == "/");
        assert(fs::path("///привет").root_path() == "/");
    }
}
