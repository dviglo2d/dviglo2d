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
    static fs::path log_path() { return get_pref_path("dviglo2d", "minimal_app") / "log.log"; }

    Config();
    ~Config() final;

    void save();
    void load();
};

#define CONFIG (Config::instance())
