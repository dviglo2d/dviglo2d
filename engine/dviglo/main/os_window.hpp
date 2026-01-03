// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "config_base.hpp"

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include <vulkan/vulkan.hpp>


namespace dviglo
{

class OsWindow final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static OsWindow* instance_ = nullptr;

    // Конструктор не может возвращать значений, поэтому используем флаг
    bool is_invalid_ = false;

    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;
    vk::UniqueInstance vk_instance_;
    vk::UniqueDebugUtilsMessengerEXT vk_debug_messenger_;
    VkSurfaceKHR vk_surface_ = nullptr;
    VkPhysicalDevice vk_physical_device_ = nullptr;
    VkDevice vk_device_ = nullptr;
    VkQueue vk_queue_ = nullptr;

    bool vk_create_instance();
    bool vk_pick_physical_device();
    bool vk_create_logical_device();

public:
    static OsWindow* instance() { return instance_; }

    OsWindow(const ConfigBase& config);
    ~OsWindow() final;

    bool is_invalid() const { return is_invalid_; }

    SDL_Window* window() const { return window_; }
    SDL_GLContext gl_context() const { return gl_context_; }

    glm::ivec2 get_size_in_pixels() const;
};

} // namespace dviglo

#define DV_OS_WINDOW (dviglo::OsWindow::instance())
