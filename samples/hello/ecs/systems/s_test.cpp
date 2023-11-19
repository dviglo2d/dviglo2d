// Copyright (c) 2022-present the Dviglo project
// License: MIT

#include "s_test.hpp"

#include <iostream>


void STest::update(entt::registry& ecs, u64 ns)
{
    auto view = ecs.view<CName, CCounter>();

    for (auto [entity, name, counter] : view.each())
    {
        ++counter.value;

        cout << "Имя сущности: " << name.value << " | Счётчик: " << counter.value
             << " | Интервал времени: " << ns << " нс" << endl;
    }
}
