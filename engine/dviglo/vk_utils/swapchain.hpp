// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_primitive_types.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>


namespace dviglo
{

class Swapchain
{
private:
    // Поля расположены в порядке инициализации

    vk::PresentModeKHR   present_mode_ = vk::PresentModeKHR::eImmediate;
    vk::Extent2D         image_extent_ = {};
    vk::SurfaceFormatKHR surface_format_ = {};

    vk::UniqueSwapchainKHR value_ = {};

    // Изображения уничтожать не надо, будут уничтожены при уничтожении value_
    std::vector<vk::Image> images_ = {};

    std::vector<vk::UniqueImageView> image_views_ = {};

    Swapchain() = default;

public:
    static std::optional<Swapchain> construct(vk::PhysicalDevice physical_device, vk::Device device,
                                              u32 graphics_queue_family_index, u32 present_queue_family_index,
                                              vk::SurfaceKHR surface, glm::ivec2 window_size,
                                              vk::PresentModeKHR present_mode);

    // Запрещаем копировать объект
    Swapchain(const Swapchain&)             = delete;
    Swapchain& operator =(const Swapchain&) = delete;

    // Но разрешаем перемещать
    Swapchain(Swapchain&& other)             noexcept = default;
    Swapchain& operator =(Swapchain&& other) noexcept = default;

    vk::PresentModeKHR present_mode()                     const { return present_mode_; }
    vk::Extent2D image_extent()                           const { return image_extent_; }
    vk::SurfaceFormatKHR surface_format()                 const { return surface_format_; }
    vk::SwapchainKHR get()                                const { return *value_; }
    const std::vector<vk::UniqueImageView>& image_views() const { return image_views_; }
};

}  // namespace dviglo
