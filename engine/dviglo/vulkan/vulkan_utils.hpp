// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_fs.hpp>
#include <glm/glm.hpp>
#include <vk_mem_alloc.hpp>


namespace dviglo
{

constexpr StrAscii vk_version_to_string(u32 version)
{
    u32 variant = vk::apiVersionVariant(version);
    u32 major   = vk::apiVersionMajor(version);
    u32 minor   = vk::apiVersionMinor(version);
    u32 patch   = vk::apiVersionPatch(version);

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

using VmaAllocatedBuffer = std::pair<vma::UniqueAllocation, vma::UniqueBuffer>;
using VmaAllocatedImage  = std::pair<vma::UniqueAllocation, vma::UniqueImage>;

constexpr vk::Extent2D to_vk_Extent2D(glm::uvec2 value)
{
    return vk::Extent2D
    {
        .width = value.x,
        .height = value.y
    };
}

constexpr vk::Extent2D to_vk_Extent2D(vk::Extent3D extent_3d)
{
    return vk::Extent2D
    {
        .width = extent_3d.width,
        .height = extent_3d.height,
    };
}

}  // namespace dviglo
