// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "app.h"

#include <dviglo/main/main.h>

#include <iostream>


i32 run(const vector<StrUtf8>& args)
{
    cout << "Командная строка: ";

    for (const StrUtf8& arg : args)
        cout << arg << " ";

    cout << endl;

    unique_ptr<App> app = make_unique<App>();
    return app->run();
}

DV_DEFINE_MAIN(run);
