#include "config.hpp"

#include <dviglo/gl_utils/texture_cache.hpp>

using namespace glm;

Config::Config()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    window_size_ = ivec2(800, 800);
    window_mode_ = WindowMode::windowed;
    //msaa_samples_ = 8; // При включении крэшится на сервере ГитХаба в Линуксе

    Texture::default_params.min_filter = GL_LINEAR_MIPMAP_LINEAR;
    Texture::default_params.mag_filter = GL_LINEAR;
}

Config::~Config()
{
    instance_ = nullptr;
}
