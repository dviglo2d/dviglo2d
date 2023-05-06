// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#pragma once

#ifdef _WIN32

    #include "../common/win_wrapped.h"

    #include <clocale> // std::setlocale

#endif // def _WIN32

#include "../std_utils/str.h"


namespace dviglo
{

#ifdef _WIN32
DV_API std::vector<StrUtf8> get_win_command_line_args();
#endif

} // namespace dviglo

#ifdef _WIN32

    #ifdef DV_WIN32_CONSOLE

        #define DV_DEFINE_MAIN(func) \
            int main(int argc, char* argv[]) \
            { \
                /* Позволяет выводить UTF-8 в консоль */ \
                std::setlocale(LC_CTYPE, "en_US.UTF-8"); \
                \
                /* Не используем argv, чтобы не было проблем с кодировкой */ \
                return func(get_win_command_line_args()); \
            }

    #else

        #define DV_DEFINE_MAIN(func) \
            int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, \
                               _In_ LPSTR lpCmdLine, _In_ int nShowCmd) \
            { \
                return func(get_win_command_line_args()); \
            }

    #endif // def DV_WIN32_CONSOLE

#else // Linux

    #define DV_DEFINE_MAIN(func) \
        int main(int argc, char* argv[]) \
        { \
            std::vector<dvt::StrUtf8> args; \
            args.reserve(argc); \
            \
            for (int i = 0; i < argc; ++i) \
                args.push_back(argv[i]); \
            \
            return func(args); \
        }

#endif // def _WIN32
