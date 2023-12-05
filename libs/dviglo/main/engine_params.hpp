// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"

#include <glm/glm_wrapped.hpp>


namespace dviglo
{

/// Эти параметры используются при инициализации движка
namespace engine_params
{
    extern StrUtf8 window_title;
    extern glm::ivec2 window_size;
}

} // namespace dviglo
