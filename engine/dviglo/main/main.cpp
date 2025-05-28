// Copyright (c) the Dviglo project
// License: MIT

#include "main.hpp"

#ifdef DV_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include "../common/windows_wrapped.hpp" // MessageBoxA

    #include <filesystem> // std::filesystem::path
    #include <iostream> // std::cerr
#endif

using namespace std;



namespace dviglo
{

bool set_locale()
{
#ifdef DV_WINDOWS
    setlocale(LC_CTYPE, ".UTF-8");

    // Windows 10 поддерживает UTF-8 начиная с версии 1803 (10.0.17134.0)
    // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170#utf-8-support
    if (filesystem::path("тест").native() != L"тест")
    {
        const char* msg = "This application requires Windows 10 version 1803 (10.0.17134.0) or later";
        cerr << msg << endl;
        MessageBoxA(nullptr, msg, "Error", MB_ICONERROR | MB_OK);
        return false;
    }

    return true;
#elif defined(DV_LINUX)
    return true;
#else
    #error
#endif
}

vector<StrUtf8> main_args_to_vector(i32 argc, char* argv[])
{
    vector<StrUtf8> ret;
    ret.reserve(argc);

    for (i32 i = 0; i < argc; ++i)
        ret.push_back(argv[i]);

    return ret;
}

} // namespace dviglo
