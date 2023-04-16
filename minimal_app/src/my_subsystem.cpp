#include "my_subsystem.hpp"


MySubsystem::MySubsystem()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    // Ваш код
}

MySubsystem::~MySubsystem()
{
    instance_ = nullptr;

    // Ваш код
}
