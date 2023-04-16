#include "config.hpp"

#include <dv_sdl_utils.hpp>

using namespace glm;


Config::Config()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    window_size_ = ivec2(900, 700);
    window_title_ = "Леталка";
    window_mode_ = WindowMode::fullscreen;

#if 1
    vsync_ = -1;
#else
    max_fps(200.0);
#endif
}

Config::~Config()
{
    instance_ = nullptr;
}
