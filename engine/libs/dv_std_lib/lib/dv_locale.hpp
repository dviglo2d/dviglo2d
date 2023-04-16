// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <locale>


namespace dviglo
{

// Включает поддержку UTF-8 в Windows для консоли и файловых путей (но не для функций типа MessageBoxA).
// Требуется Windows 10 версии 1803 (10.0.17134.0) или выше:
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-170#utf-8-support
bool set_utf8_locale();

} // namespace dviglo
