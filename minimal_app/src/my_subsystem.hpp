#pragma once

#include <dv_subsystem_base.hpp>

using namespace dviglo;


class MySubsystem final : public SubsystemBase<MySubsystem>
{
public:
    MySubsystem();
    ~MySubsystem();

    // Ваш метод
    i32 get_value() const { return 1; }
};

#define MY_SUBSYSTEM (MySubsystem::instance())
