// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "app.h"


App::App()
{
    log_path_ = "путь/к/логу";
}

void App::start()
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

void App::update(u64 ms)
{
    s_test_.update(ecs_, ms);
}
