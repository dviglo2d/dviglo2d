// Copyright (c) the Dviglo project
// License: MIT

// Этот файл используется в тестерах, чтобы assert(...) работал даже в релизных конфигурациях.
// Файл нужно подключать вместо/перед <cassert>

#pragma once

#ifdef assert
    #error "Don't include <cassert>"
#endif

// Макрос NDEBUG отключает assert(...): https://en.cppreference.com/w/cpp/error/assert
// Этот макрос определён в конфигурациях Release, RelWithDebInfo, MinSizeRel
// и не определён в конфигурации Debug
#undef NDEBUG

#include <cassert>
