// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "config_base.hpp"
#include "../vulkan/swapchain.hpp"

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include <vk_mem_alloc.hpp>


namespace dviglo
{

class Graphics final : public SubsystemIndex
{
private:
    friend class ApplicationBase;

    // Инициализируется в конструкторе
    inline static Graphics* instance_ = nullptr;

    bool is_valid_ = false;

    // ============== Поля упорядочены в порядке инициализации ==============

    std::vector<vk::ExtensionProperties> vk_instance_extensions_; // Доступные расширения для VkInstance
    std::vector<vk::LayerProperties> vk_layers_; // Доступные слои валидации
    SDL_Window* window_ = nullptr;

    vk::UniqueInstance vk_instance_;
    bool vk_create_instance();

    vk::UniqueDebugUtilsMessengerEXT vk_debug_utils_messenger_;
    vk::UniqueSurfaceKHR vk_surface_;

    // Эти два семейства очередей скорее всего будут одним семейством
    u32 vk_graphics_queue_family_index_ = std::numeric_limits<u32>::max();
    u32 vk_present_queue_family_index_ = std::numeric_limits<u32>::max();
    vk::PhysicalDevice vk_physical_device_;
    bool vk_pick_physical_device(const std::vector<vk::PhysicalDevice>& physical_devices);

    vk::UniqueDevice vk_device_; // Логическое устройство
    vk::Queue vk_graphics_queue_;
    vk::Queue vk_present_queue_;
    bool vk_create_device();

    // Командный пул, который никогда не сбрасывается.
    // Выделенные из него буферы существуют до завершения программы
    vk::UniqueCommandPool persistent_command_pool_;

    // Командный пул, который сбрасывается каждый кадр
    vk::UniqueCommandPool transient_command_pool_;

    vma::UniqueAllocator vma_allocator_;
    std::unique_ptr<Swapchain> swapchain_;

    u32 graphics_queue_index_ = 0; // TODO

public:
    static Graphics* instance() { return instance_; }

    Graphics(const ConfigBase& config);
    ~Graphics() final;

    bool is_valid() const { return is_valid_; }

    vk::Device device() const { return *vk_device_; }

    vma::Allocator vma_allocator() const { return *vma_allocator_; }

    vk::Queue graphics_queue() const { return vk_present_queue_; }
    u32 graphics_queue_index() const { return graphics_queue_index_; }

    // Командный пул, который никогда не сбрасывается.
    // Выделенные из него буферы существуют до завершения программы
    vk::CommandPool persistent_command_pool() const { return *persistent_command_pool_; }

    // Командный пул, который сбрасывается каждый кадр
    vk::CommandPool transient_command_pool() const { return *transient_command_pool_; }

    SDL_Window* window() const { return window_; }

    glm::ivec2 get_size_in_pixels() const;
};

} // namespace dviglo

#define DV_GRAPHICS (dviglo::Graphics::instance())
