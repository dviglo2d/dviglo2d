#pragma once

#include <dv_subsystem_index.hpp>

using namespace dviglo;


class MySubsystem final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static MySubsystem* instance_ = nullptr;

public:
    static MySubsystem* instance() { return instance_; }

    MySubsystem();
    ~MySubsystem() final;

    // Ваш метод
    i32 get_value() const { return 1; }
};

#define MY_SUBSYSTEM (MySubsystem::instance())
