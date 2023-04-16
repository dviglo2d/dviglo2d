#include "my_subsystem.hpp"

#include <dv_log.hpp>


MySubsystem::MySubsystem()
{
    // Ваш код

    instance_ = this;
    Log::write_debug("MySubsystem constructed");
}

MySubsystem::~MySubsystem()
{
    // Ваш код
}
