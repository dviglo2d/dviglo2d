#pragma once

#include <dviglo/main/config_base.hpp>

using namespace dviglo;


class Config final : public ConfigBase
{
private:
    // Инициализируется в конструкторе
    inline static Config* instance_ = nullptr;

public:
    static Config* instance() { return instance_; }
    static fs::path get_log_path() { return get_pref_path("dviglo2d", "games") / "survivors.log"; }

    Config();
    ~Config() final;

    void save();
    void load();
};

#define CONFIG (Config::instance())
