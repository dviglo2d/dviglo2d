// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "force_assert.h"

#include <iostream>

using namespace std;


void run()
{
    assert(1 == 1);
}

int main(int argc, char* argv[])
{
    std::setlocale(LC_CTYPE, "en_US.UTF-8");

    run();

    cout << "Все тесты пройдены успешно" << endl;

    return 0;
}
