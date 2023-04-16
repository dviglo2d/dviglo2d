// Copyright (c) the Dviglo project
// License: MIT

#include <iostream>

using namespace std;


void test_io_path();
void test_std_utils_str();

void run()
{
    test_io_path();
    test_std_utils_str();
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    setlocale(LC_CTYPE, "en_US.UTF-8");
    run();
    cout << "Все тесты пройдены успешно" << endl;

    return 0;
}
