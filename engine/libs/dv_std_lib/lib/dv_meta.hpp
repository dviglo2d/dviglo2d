// Copyright (c) the Dviglo project
// License: MIT

// Полноценная рефлексия будет в C++26:
// https://isocpp.org/files/papers/P2996R4.html
// https://habr.com/ru/articles/824840/

#pragma once

#include "dv_string.hpp"


// В MSVC выглядит         void __cdecl App::update(__int64)
// В GCC и MinGW выглядит  void App::update(dvt::i64)
// В Clang выглядит        void App::update(i64)
#if defined(DV_WINDOWS_MSVC)
    #define DV_FUNC_SIG dvt::StrUtf8(__FUNCSIG__)
#elif defined(DV_LINUX_GCC) || defined(DV_LINUX_CLANG) || defined(DV_WINDOWS_MINGW)
    #define DV_FUNC_SIG dvt::StrUtf8(__PRETTY_FUNCTION__)
#else
    #error
#endif
