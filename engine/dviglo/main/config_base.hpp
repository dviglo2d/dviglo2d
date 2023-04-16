// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_fs.hpp>
#include <dv_subsystem_index.hpp>


namespace dviglo
{

class ConfigBase : public SubsystemIndex
{
public:
    // Пользователь может переопределить этот метод.
    // Метод статический, так как текущая подсистема содаётся после открытия лога
    static fs::path get_log_path() { return get_pref_path("dviglo2d", "default") / "log.log"; }
};

} // namespace dviglo
