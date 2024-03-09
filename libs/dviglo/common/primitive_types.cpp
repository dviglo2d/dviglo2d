// Copyright (c) the Dviglo project
// License: MIT

#include <cstddef>     // ptrdiff_t
#include <cstdint>     // int64_t
#include <limits>      // numeric_limits
#include <type_traits> // is_same_v

using namespace std;


// https://en.cppreference.com/w/cpp/language/types
static_assert(numeric_limits<unsigned char>::digits == 8);
static_assert(sizeof(short) == 2);
static_assert(sizeof(int) == 4);
static_assert(sizeof(long long) == 8);
static_assert(sizeof(char32_t) == 4);

#ifdef _WIN32 // MSVC или MinGW
static_assert(sizeof(long) == 4);
static_assert(sizeof(wchar_t) == 2);
static_assert(is_same_v<int64_t, long long>);
#else // GCC или Clang
static_assert(sizeof(long) == sizeof(void*)); // 4 или 8
static_assert(sizeof(wchar_t) == 4);
static_assert((sizeof(void*) == 4 && is_same_v<int64_t, long long>)
           || (sizeof(void*) == 8 && is_same_v<int64_t, long>));
#endif

// Тип 0x7FFFFFFFFFFFFFFF зависит от компилятора
static_assert(is_same_v<decltype(0x7FFFFFFFFFFFFFFF), int64_t>);

// Арифметика указателей
static_assert(sizeof(void*) == sizeof(ptrdiff_t));
static_assert(sizeof(void*) == sizeof(intptr_t));

// Dviglo поддерживает только 64-битные платформы
static_assert(sizeof(void*) == 8);
