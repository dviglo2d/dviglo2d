// Copyright (c) the Dviglo project
// License: MIT

#include "main.hpp"

#ifdef _WIN32
    #include <clocale> // std::setlocale
#endif

using namespace std;


namespace dviglo
{

vector<StrUtf8> main_args_to_vector(i32 argc, char* argv[])
{
#ifdef _WIN32
    // Функция main_args_to_vector() вызывается первой в main(), поэтому
    // меняем кодировку консоли здесь
    setlocale(LC_CTYPE, "en_US.UTF-8");
#endif

    vector<StrUtf8> ret;
    ret.reserve(argc);

    for (i32 i = 0; i < argc; ++i)
        ret.push_back(argv[i]);

    return ret;
}

} // namespace dviglo
