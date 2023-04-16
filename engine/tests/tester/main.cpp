#include <dv_force_assert.hpp>

#include <dv_locale.hpp>
#include <iostream>

using namespace std;


void test_res_freetype_misc();

void run()
{
    test_res_freetype_misc();
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    assert(dviglo::set_utf8_locale());

    run();
    cout << "Все тесты пройдены успешно" << endl;

    return 0;
}
