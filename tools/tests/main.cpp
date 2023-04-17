// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <iostream>

using namespace std;


void test_std_utils_str();

void run()
{
    test_std_utils_str();
}

int main(int argc, char* argv[])
{
    std::setlocale(LC_CTYPE, "en_US.UTF-8");

    run();

    cout << "Все тесты пройдены успешно" << endl;

    return 0;
}
