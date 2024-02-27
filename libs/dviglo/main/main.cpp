// Copyright (c) the Dviglo project
// License: MIT

#include "main.hpp"

#ifdef _WIN32

    #include "../common/win_wrapped.hpp"

    #include <clocale> // std::setlocale

#endif

using namespace std;


namespace dviglo
{

#ifdef _WIN32

// Не используем argv, чтобы не было проблем с кодировкой в консольной версии приложения
vector<StrUtf8> get_command_line_args(i32 /*argc*/, char*[] /*argv*/)
{
    // Функция get_command_line_args() вызывается первой в main(), поэтому
    // меняем кодировку консоли здесь
    std::setlocale(LC_CTYPE, "en_US.UTF-8");

    vector<StrUtf8> ret;

    i32 num_args;
    LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &num_args);

    if (!args)
        return ret;

    ret.reserve(num_args);

    for (i32 i = 0; i < num_args; ++i)
        ret.push_back(from_wstring(args[i]));

    LocalFree(args);

    return ret;
}

#else // Linux

vector<StrUtf8> get_command_line_args(i32 argc, char* argv[])
{
    std::vector<dvt::StrUtf8> args;
    args.reserve(argc);

    for (i32 i = 0; i < argc; ++i)
        args.push_back(argv[i]);

    return args;
}

#endif // def _WIN32

} // namespace dviglo
