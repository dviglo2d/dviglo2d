// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "config_base.hpp"

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include <vk_mem_alloc.hpp>


namespace dviglo
{

class OsWindow final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static OsWindow* instance_ = nullptr;

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
    u32 vk_graphics_family_index_ = std::numeric_limits<u32>::max();
    u32 vk_present_family_index_ = std::numeric_limits<u32>::max();
    vk::Queue vk_graphics_queue_;
    vk::Queue vk_present_queue_;

    vk::PhysicalDevice vk_physical_device_;
    bool vk_pick_physical_device(const std::vector<vk::PhysicalDevice>& physical_devices);

    vk::UniqueDevice vk_device_; // Логическое устройство
    bool vk_create_device();

    vma::UniqueAllocator vma_allocator_;
    vk::UniqueCommandPool vk_command_pool_;

public:
    static OsWindow* instance() { return instance_; }

    OsWindow(const ConfigBase& config);
    ~OsWindow() final;

    bool is_valid() const { return is_valid_; }

    SDL_Window* window() const { return window_; }

    glm::ivec2 get_size_in_pixels() const;
};

} // namespace dviglo

#define DV_OS_WINDOW (dviglo::OsWindow::instance())
