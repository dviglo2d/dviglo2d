// Copyright (c) the Dviglo project
// License: MIT

// Этот файл нужно подключать вместо/перед <stb_image_write.h>.
// Подавляет предупреждения

#pragma once

#ifdef INCLUDE_STB_IMAGE_WRITE_H
    #error "Don't include <stb_image_write.h>"
#endif

#include <dv_platform.hpp>

#if defined(DV_LINUX_GCC) || defined(DV_LINUX_CLANG) || defined(DV_WINDOWS_MINGW)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include <stb_image_write.h>

#if defined(DV_LINUX_GCC) || defined(DV_LINUX_CLANG) || defined(DV_WINDOWS_MINGW)
    #pragma GCC diagnostic pop
#endif
