// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "ecs/systems/s_test.h"

#include <dviglo/gl_utils/shader_program.h>
#include <dviglo/main/application.h>

using namespace std;


class App : public Application
{
    entt::registry ecs_;
    STest s_test_;
    unique_ptr<ShaderProgram> basic_shader_;

public:
    App(const vector<StrUtf8>& args);

    entt::registry& ecs() { return ecs_; }

    void start() override;
    void update(u64 ms) override;
    void draw() override;
};
