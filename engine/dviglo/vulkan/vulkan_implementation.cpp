// Copyright (c) the Dviglo project
// License: MIT

#include <vulkan/vulkan.hpp>

static_assert(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC);
// Хранилище для указателей на функции Vulkan
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.hpp>
