// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <dviglo/main/application.h>
#include <dviglo/main/main.h>

#include <memory>

using namespace dviglo;
using namespace std;


class App : public Application
{
public:
    App()
    {
        log_path_ = "путь/к/логу";
    }
};


i32 run()
{
    unique_ptr<App> app = make_unique<App>();
    return app->run();
}

DV_DEFINE_MAIN(run);
