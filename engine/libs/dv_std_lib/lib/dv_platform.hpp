// Copyright (c) the Dviglo project
// License: MIT

#pragma once


// Определяем OS
#if defined(_WIN32)
    #define DV_WINDOWS 1 // MSVC или MinGW
#elif defined(__linux__)
    #define DV_LINUX 1 // GCC или Clang
#else
    #error
#endif

// Определяем компилятор
#if defined(DV_WINDOWS)
    #if defined(__MINGW32__)
        #define DV_WINDOWS_MINGW 1
    #elif defined(_MSC_VER)
        #define DV_WINDOWS_MSVC 1 // Microsoft Visual C++
    #else
        #error
    #endif
#elif defined(DV_LINUX)
    #if defined(__clang__)
        #define DV_LINUX_CLANG 1
        static_assert(__clang_major__ >= 18);
    #elif defined(__GNUC__)
        #define DV_LINUX_GCC 1
        static_assert(__GNUC__ >= 13);
    #else
        #error
    #endif
#else
    #error
#endif

#if defined(DV_OPENMP) && !defined(_OPENMP)
    #error "OpenMP is required"
#endif
