// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "main.hpp"

using namespace std;


namespace dviglo
{

#ifdef _WIN32

vector<StrUtf8> get_win_command_line_args()
{
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

#endif

} // namespace dviglo
