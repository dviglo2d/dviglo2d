// Copyright (c) the Dviglo project
// License: MIT

// Нужно подключать этот файл вместо <windows.h>.
// Чтобы узнать, какой файл подключает windows.h без обёртки, можно воспользоваться опцией VS:
// ПКМ по проекту -> Properties -> Configuration Properties -> C/C++ -> Advanced -> Show Includes

#pragma once

#ifdef _WINDOWS_
    #error "Don't include <windows.h>"
#endif

// В файле rpcndr.h определён тип byte, который конфликтует с byte движка
#define byte BYTE

#include <windows.h>

#undef byte
#undef max
