// Copyright (c) the Dviglo project
// License: MIT

#include <dv_force_assert.hpp>

#include <dv_locale.hpp>
#include <iostream>


void test_flags();
void test_fs();
void test_math();
void test_primitive_types();
void test_string();

void run()
{
    test_flags();
    test_fs();
    test_math();
    test_primitive_types();
    test_string();
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    assert(dviglo::set_utf8_locale());

    run();
    std::cout << "Все тесты пройдены успешно" << std::endl;

    return 0;
}
