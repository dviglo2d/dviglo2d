#include "config.hpp"

using namespace glm;


Config::Config()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    window_title_ = "Игра";
    window_size_ = ivec2(900, 700);
    //msaa_samples_ = 8; // При включении крэшится на сервере ГитХаба в Линуксе
    window_mode_ = WindowMode::windowed;

    load();
}

Config::~Config()
{
    instance_ = nullptr;

    save();
}

void Config::save()
{
    /// TODO
}

void Config::load()
{
    // TODO
}
