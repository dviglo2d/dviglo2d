// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "ecs/systems/s_test.h"

#include <dviglo/main/application.h>


class App : public Application
{
    entt::registry ecs_;
    STest s_test_;

public:
    App();

    entt::registry& ecs() { return ecs_; }

    void start() override;
    void update() override;
};
