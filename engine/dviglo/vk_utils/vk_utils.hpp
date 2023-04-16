// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_fs.hpp>
#include <vk_mem_alloc.hpp>


namespace dviglo
{

constexpr StrAscii vk_version_to_string(u32 version)
{
    u32 variant = vk::apiVersionVariant(version);
    u32 major = vk::apiVersionMajor(version);
    u32 minor = vk::apiVersionMinor(version);
    u32 patch = vk::apiVersionPatch(version);

    // Если версия - просто число
    if (!variant && !major && !minor)
        return std::to_string(patch);

    StrAscii ret;

    if (variant)
        ret = std::to_string(variant) + ".";

    ret += std::to_string(vk::apiVersionMajor(version))
           + "." + std::to_string(vk::apiVersionMinor(version))
           + "." + std::to_string(vk::apiVersionPatch(version));

    return ret;
}

constexpr bool contains(const std::vector<vk::ExtensionProperties>& extensions, const StrAscii& extension_name)
{
    for (const vk::ExtensionProperties& extension : extensions)
    {
        if (extension.extensionName.data() == extension_name)
            return true;
    }

    return false;
}

constexpr bool contains(const std::vector<vk::LayerProperties>& layers, const StrAscii& layer_name)
{
    for (const vk::LayerProperties& layer : layers)
    {
        if (layer.layerName.data() == layer_name)
            return true;
    }

    return false;
}

// Загружает SPIR-V шейдер из файла. Возвращает {} в случае неудачи
vk::UniqueShaderModule load_shader(const vk::Device device, const fs::path& path);

// В vulkan.hpp пока нет специализации для std::pair
template <typename T1, typename T2, typename Dispatch>
std::tuple<vk::Result, std::pair<vk::UniqueHandle<T1, Dispatch>, vk::UniqueHandle<T2, Dispatch>>>
    as_tuple(vk::ResultValue<std::pair<vk::UniqueHandle<T1, Dispatch>, vk::UniqueHandle<T2, Dispatch>>>&& rv)
{
    return std::make_tuple(rv.result, std::move(rv.value));
}

using VmaAllocatedBuffer = std::pair<vma::UniqueAllocation, vma::UniqueBuffer>;

}  // namespace dviglo
