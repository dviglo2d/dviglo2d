#include "config.hpp"

using namespace glm;


Config::Config()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    window_size_ = ivec2(1100, 600);
    window_mode_ = WindowMode::maximized;
    window_title_ = "Sprite Manipulator";

}

Config::~Config()
{
    instance_ = nullptr;
}
