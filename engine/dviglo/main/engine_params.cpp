// Copyright (c) the Dviglo project
// License: MIT

#include "engine_params.hpp"

#include "../fs/fs_base.hpp"


namespace dviglo
{

namespace engine_params
{
    StrUtf8 log_path = get_pref_path("", "dviglo2d") + "default.log";
}

} // namespace dviglo
