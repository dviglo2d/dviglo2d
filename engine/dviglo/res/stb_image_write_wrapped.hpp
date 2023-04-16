// Нужно подключать этот файл вместо <stb_image_write.h>.
// Подавляет предупреждение missing initializer for member 'stbi__write_context::context'.
// https://github.com/nothings/stb/issues/1709

#pragma once

#ifdef INCLUDE_STB_IMAGE_WRITE_H
    #error "Don't include <stb_image_write.h>"
#endif

#if defined(__GNUC__) // GCC, Clang или MinGW
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include <stb_image_write.h>

#if defined(__GNUC__) // GCC, Clang или MinGW
    #pragma GCC diagnostic pop
#endif
