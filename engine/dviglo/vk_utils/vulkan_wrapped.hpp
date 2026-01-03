// Copyright (c) the Dviglo project
// License: MIT

// Этот файл нужно подключать вместо/перед <vulkan/vulkan.hpp>

#pragma once

#ifdef VULKAN_HPP
    #error "Don't include <vulkan/vulkan.hpp>"
#endif

#define VULKAN_HPP_NO_CONSTRUCTORS  // Убираем конструкторы, чтобы разрешить aggregate initialization
#define VULKAN_HPP_NO_EXCEPTIONS    // Отключаем исключения
#include <vulkan/vulkan.hpp>
