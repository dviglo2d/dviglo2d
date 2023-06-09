// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"

#include <glm/glm.hpp>


namespace dviglo
{

/// Эти параметры используются при инициализации движка
namespace engine_params
{
    DV_API extern StrUtf8 window_title;
    DV_API extern glm::ivec2 window_size;
}

} // namespace dviglo
