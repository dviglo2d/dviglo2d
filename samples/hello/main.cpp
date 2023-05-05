// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "ecs/components.h"
#include "ecs/systems/s_test.h"

#include <dviglo/main/application.h>
#include <dviglo/main/main.h>

#include <memory>

using namespace dviglo;
using namespace std;


class App : public Application
{
    entt::registry ecs_;

    STest s_test_;

public:
    App()
    {
        log_path_ = "путь/к/логу";
    }

    entt::registry& ecs() { return ecs_; }

    void start() override
    {
        entt::entity ent1 = ecs_.create();

        CName& ent1_name = ecs_.emplace<CName>(ent1);
        ent1_name.value = "entity 1";

        CCounter& ent1_counter = ecs_.emplace<CCounter>(ent1);
        ent1_counter.value = 10;

        entt::entity ent2 = ecs_.create();

        CName& ent2_name = ecs_.emplace<CName>(ent2);
        ent2_name.value = "entity 2";

        CCounter& ent2_counter = ecs_.emplace<CCounter>(ent2);
        ent2_counter.value = 100;
    }

    void update() override
    {
        s_test_.update(ecs_);
    }
};


i32 run()
{
    unique_ptr<App> app = make_unique<App>();
    return app->run();
}

DV_DEFINE_MAIN(run);
