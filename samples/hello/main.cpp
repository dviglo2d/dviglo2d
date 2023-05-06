// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "app.h"

#include <dviglo/main/main.h>


i32 run()
{
    unique_ptr<App> app = make_unique<App>();
    return app->run();
}

DV_DEFINE_MAIN(run);
