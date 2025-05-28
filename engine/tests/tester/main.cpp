// Copyright (c) the Dviglo project
// License: MIT

#include "force_assert.hpp"

#include <dviglo/main/main.hpp>

#include <iostream>

using namespace std;


void test_common_primitive_types();
void test_fs_path();
void test_math_math();
void test_res_freetype_utils();
void test_std_utils_flags();
void test_std_utils_string();

void run()
{
    test_common_primitive_types();
    test_fs_path();
    test_math_math();
    test_res_freetype_utils();
    test_std_utils_flags();
    test_std_utils_string();
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    assert(dviglo::set_locale());

    run();
    cout << "Все тесты пройдены успешно" << endl;

    return 0;
}
