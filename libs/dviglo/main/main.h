// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#pragma once

#if defined _WIN32 && !defined DV_WIN32_CONSOLE
#include "../common/win_wrapped.h"
#endif

#include <clocale>


#if defined _WIN32 && !defined DV_WIN32_CONSOLE

    #define DV_DEFINE_MAIN(func) \
        int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, \
                           _In_ LPSTR lpCmdLine, _In_ int nShowCmd) \
        { \
            /* Позволяет выводить UTF-8 в консоль в Windows. В Linux консоль в кодировке UTF-8 по умолчанию */ \
            std::setlocale(LC_CTYPE, "en_US.UTF-8"); \
            \
            return func(); \
        }

#else // Linux или консольное приложение Windows

    #define DV_DEFINE_MAIN(func) \
        int main(int argc, char* argv[]) \
        { \
            /* Позволяет выводить UTF-8 в консоль в Windows. В Linux консоль в кодировке UTF-8 по умолчанию */ \
            std::setlocale(LC_CTYPE, "en_US.UTF-8"); \
            \
            return func(); \
        }

#endif
