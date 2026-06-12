// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "vulkan_image.hpp"

#include <dv_subsystem_index.hpp>


namespace dviglo
{

// Все текстуры должны храниться в кэше, чтобы гарантировать их уничтожение
// перед уничтожением vk::Device
class TextureCacheNew final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static TextureCacheNew* instance_ = nullptr;

    // Отдельный командный пул для пересылки изображений в видеопамять
    vk::UniqueCommandPool vk_command_pool_;

    vma::Allocator vma_allocator_;
    bool is_valid_ = false;

    std::unique_ptr<VulkanImage> white_pixel_;

public:
    static TextureCacheNew* instance() { return instance_; }

    TextureCacheNew(vma::Allocator vma_allocator, vk::Queue queue, u32 queue_family_index);
    ~TextureCacheNew() override;

    bool is_valid() const { return is_valid_; }
};

} // namespace dviglo

#define DV_TEXTURE_CACHE_NEW (dviglo::TextureCacheNew::instance())
