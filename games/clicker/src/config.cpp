#include "config.hpp"


Config::Config()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;
}

Config::~Config()
{
    instance_ = nullptr;
}

void Config::save()
{
}

void Config::load()
{
}
