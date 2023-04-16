// Copyright (c) the Dviglo project
// License: MIT

#include "main.hpp"

#include <dv_locale.hpp>
#include <iostream> // std::cerr

#if DV_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <dv_windows_wrapped.hpp> // MessageBoxA
#endif

using namespace std;



namespace dviglo
{

bool set_locale()
{
    bool ret = set_utf8_locale();

#if DV_WINDOWS
    // Windows 10 поддерживает UTF-8 начиная с версии 1803 (10.0.17134.0)
    // https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170#utf-8-support
    if (!ret)
    {
        const char* msg = "This application requires Windows 10 version 1803 (10.0.17134.0) or later";
        cerr << msg << endl;
        MessageBoxA(nullptr, msg, "Error", MB_ICONERROR | MB_OK);
    }
#elif DV_LINUX
    if (!ret)
    {
        const char* msg = "Error | set_locale() | !ret";
        cerr << msg << endl;
    }
#else
    #error
#endif

    return ret;
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
