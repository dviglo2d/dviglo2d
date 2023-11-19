// Copyright (c) the Dviglo project
// License: MIT

#include "app.hpp"

#include <dviglo/main/main.hpp>


i32 app_main(const vector<StrUtf8>& args)
{
    unique_ptr<App> app = make_unique<App>(args);
    return app->run();
}

DV_DEFINE_MAIN(app_main);
