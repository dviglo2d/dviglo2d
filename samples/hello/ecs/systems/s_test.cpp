// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "s_test.h"

#include <iostream>

void STest::update(entt::registry& ecs)
{
    auto view = ecs.view<CName, CCounter>();

    for (entt::entity entity : view)
    {
        auto [name, counter] = view.get(entity);

        ++counter.value;

        cout << "Имя сущности: " << name.value << " Счётчик: " << counter.value << endl;
    }
}
