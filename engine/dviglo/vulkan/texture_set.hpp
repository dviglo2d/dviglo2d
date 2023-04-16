// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "vulkan_image.hpp"

#include <dv_subsystem_index.hpp>


namespace dviglo
{

// Набор всех текстур передаётся в каждый шейдер
class TextureSet final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static TextureSet* instance_ = nullptr;

    vma::Allocator vma_allocator_;
    bool is_valid_ = false;

    VulkanImage white_pixel_;
    std::unordered_map<fs::path, VulkanImage> umap_storage_;

public:
    static TextureSet* instance() { return instance_; }

    TextureSet(vma::Allocator vma_allocator, vk::Queue queue, u32 queue_family_index);
    ~TextureSet() override;

    bool is_valid() const { return is_valid_; }

    const VulkanImage& white_pixel() const { return white_pixel_; }
    const VulkanImage& get(const fs::path& path, vk::Format format = vk::Format::eR8G8B8A8Unorm);
};

} // namespace dviglo

#define DV_TEXTURE_SET (dviglo::TextureSet::instance())
