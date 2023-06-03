// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "s_test.hpp"

#include <iostream>


void STest::update(entt::registry& ecs, u64 ms)
{
    auto view = ecs.view<CName, CCounter>();

    for (auto [entity, name, counter] : view.each())
    {
        ++counter.value;

        cout << "Имя сущности: " << name.value << " | Счётчик: " << counter.value
             << " | Интервал времени: " << ms << " мс" << endl;
    }
}
