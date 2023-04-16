// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#pragma once

#if defined _WIN32 && !defined DV_WIN32_CONSOLE
#include <Windows.h>
#endif


#if defined _WIN32 && !defined DV_WIN32_CONSOLE

    #define DV_DEFINE_MAIN(func) \
        int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) \
        { \
            return func(); \
        }

#else // Linux или консольное приложение Windows

    #define DV_DEFINE_MAIN(func) \
        int main(int argc, char* argv[]) \
        { \
            return func(); \
        }

#endif
