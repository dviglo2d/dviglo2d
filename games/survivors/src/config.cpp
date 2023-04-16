#include "config.hpp"

using namespace glm;


Config::Config()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    window_size_ = ivec2(720, 700);
    window_title_ = "Выжившие";
    vsync_ = -1;
    window_mode_ = WindowMode::maximized;
}

Config::~Config()
{
    instance_ = nullptr;
}
