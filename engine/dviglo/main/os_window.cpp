// Copyright (c) the Dviglo project
// License: MIT

#include "os_window.hpp"

#include <dv_log.hpp>
#include <dv_sdl_props.hpp>
#include <glad/gl.h>
#include <SDL3/SDL_vulkan.h>

#include <iostream>

using namespace glm;
using namespace std;



namespace dviglo
{

// Callback для вывода сообщений Vulkan
static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    Log::write_error(pCallbackData->pMessage);

    return VK_FALSE;
}

// Список обязательных расширений Vulkan.
// В Windows возвращает [VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME]
static vector<const char*> get_required_instance_extensions()
{
    u32 count;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    // Можно копировать указатели, так как строки хранятся в статической памяти
    vector<const char*> ret;
    for (u32 i = 0; i < count; ++i)
        ret.push_back(extensions[i]);

    return ret;
}

bool OsWindow::vk_create_instance()
{
    vector<const char*> extensions = get_required_instance_extensions();
    vector<const char*> layers;

    // Отладка
#if 1
    // Добавление этого выводит отладочные сообщения в консоль
    layers.push_back("VK_LAYER_KHRONOS_validation");
    extensions.push_back(vk::EXTDebugReportExtensionName);

    // Добавление этого позволяет определить callback для вывода отладочных сообщений в файл
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif

    vk::ApplicationInfo app_info{ .apiVersion = VK_API_VERSION_1_0 };

    vk::InstanceCreateInfo create_info{ .pApplicationInfo = &app_info };
    create_info.setPEnabledLayerNames(layers)
               .setPEnabledExtensionNames(extensions);

    vk::Result vk_result;
    tie(vk_result, vk_instance_) = vk::createInstanceUnique(create_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk::createInstanceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        is_invalid_ = true;
        return false;
    }

    return true;
}

bool OsWindow::vk_pick_physical_device()
{
#if 0

    // Получаем число физических устройств
    u32 gpu_count;
    VkResult vk_result = vkEnumeratePhysicalDevices(vk_instance_, &gpu_count, nullptr);
    if(vk_result != VK_SUCCESS)
    {
        Log::writef_error("{} | vkEnumeratePhysicalDevices(...) | 1 | {}", DV_FUNC_SIG, string_VkResult(vk_result));
        is_invalid_ = true;
        return false;
    }

    // Получаем список физических устройств
    vector<VkPhysicalDevice> gpus;
    gpus.resize(gpu_count);
    vk_result = vkEnumeratePhysicalDevices(vk_instance_, &gpu_count, gpus.data());
    if (vk_result != VK_SUCCESS)
    {
        Log::writef_error("{} | vkEnumeratePhysicalDevices(...) | 2 | {}", DV_FUNC_SIG, string_VkResult(vk_result));
        is_invalid_ = true;
        return false;
    }

    // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
    // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
    // dedicated GPUs) is out of scope of this sample.
    for (VkPhysicalDevice& device : gpus)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            vk_physical_device_ = device;
            return true;
        }
    }

    // Use first GPU (Integrated) is a Discrete one is not available.
    if (gpu_count > 0)
    {
        vk_physical_device_ = gpus[0];
        return true;
    }

#endif

    return false;
}

bool OsWindow::vk_create_logical_device()
{
    assert

    VULKAN_HPP_ASSERT(1 == 1);


#if 0

    vector<const char*> device_extensions;
    device_extensions.push_back("VK_KHR_swapchain");

#if 0 // Хз зачем этот код
    // Enumerate physical device extension
    uint32_t properties_count;
    vector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(vk_physical_device_, nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    vkEnumerateDeviceExtensionProperties(vk_physical_device_, nullptr, &properties_count, properties.data());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
#endif

    u32 g_QueueFamilyIndex = 0;

    const float queue_priority[] = { 1.0f };
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = g_QueueFamilyIndex;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();
    VkResult vk_result = vkCreateDevice(vk_physical_device_, &create_info, nullptr, &vk_device_);
    if (vk_result != VK_SUCCESS)
    {
        Log::writef_error("{} | vkCreateDevice(...) | {}", DV_FUNC_SIG, string_VkResult(vk_result));
        is_invalid_ = true;
        return false;
    }
    vkGetDeviceQueue(vk_device_, g_QueueFamilyIndex, 0, &vk_queue_);

#endif

    return true;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

OsWindow::OsWindow(const ConfigBase& config)
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        Log::writef_error("{} | !SDL_InitSubSystem(SDL_INIT_VIDEO) | {}", DV_FUNC_SIG, SDL_GetError());
        is_invalid_ = true;
        return;
    }

    // TODO
    /*
    if (config.msaa_samples() > 1)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, config.msaa_samples());
    }
    */

    SdlPropsId props;
    props.set_boolean(SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
    props.set_string(SDL_PROP_WINDOW_CREATE_TITLE_STRING, config.window_title());
    props.set_number(SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_UNDEFINED);
    props.set_number(SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_UNDEFINED);
    props.set_number(SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, config.window_size().x);
    props.set_number(SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, config.window_size().y);
    props.set_boolean(SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN,
                      config.window_mode() == WindowMode::resizable
                      || config.window_mode() == WindowMode::maximized);
    props.set_boolean(SDL_PROP_WINDOW_CREATE_MAXIMIZED_BOOLEAN,
                      config.window_mode() == WindowMode::maximized);
    props.set_boolean(SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN,
                      config.window_mode() == WindowMode::fullscreen
                      || config.window_mode() == WindowMode::exclusive_fullscreen);
    window_ = SDL_CreateWindowWithProperties(props.get());

    if (!window_)
    {
        Log::writef_error("{} | !window_ | {}", DV_FUNC_SIG, SDL_GetError());
        is_invalid_ = true;
        return;
    }

    if (config.window_mode() == WindowMode::exclusive_fullscreen)
    {
        SDL_DisplayMode mode;
        SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(window_),
                                            config.window_size().x, config.window_size().y,
                                            0.f, false, &mode);
        SDL_SetWindowFullscreenMode(window_, &mode);
    }

    if (!vk_create_instance())
        return;

#if 0

    // Доступно только, если при создании контекста было запрошено расширение VK_EXT_DEBUG_UTILS_EXTENSION_NAME 
#if 1
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = vk_debug_callback;
        createInfo.pUserData = nullptr; // Optional

        VkResult vk_result = CreateDebugUtilsMessengerEXT(vk_instance_, &createInfo, nullptr, &vk_debug_messenger_);
        if (vk_result != VK_SUCCESS)
        {
            Log::writef_error("{} | CreateDebugUtilsMessengerEXT(...) | {}", DV_FUNC_SIG, string_VkResult(vk_result));
            is_invalid_ = true;
            return;
        }
    }
#endif


    // Принудительная ошибка
    //vkDestroyDevice(VK_NULL_HANDLE, nullptr);

    {
        if (!SDL_Vulkan_CreateSurface(window_, vk_instance_, nullptr, &vk_surface_))
        {
            Log::writef_error("{} | !SDL_Vulkan_CreateSurface(...) | {}", DV_FUNC_SIG, SDL_GetError());
            is_invalid_ = true;
            return;
        }
    }
#endif

    // Выбираем устройство после создания поверхности, так как поверхность может повлиять на выбор устройства
    if (!vk_pick_physical_device())
        return;

    if (!vk_create_logical_device())
        return;

    gl_context_ = SDL_GL_CreateContext(window_);

    if (!gl_context_)
    {
        Log::writef_error("{} | !gl_context_ | {}", DV_FUNC_SIG, SDL_GetError());
        is_invalid_ = true;
        return;
    }

    // Настраиваем VSync
    i32 vsync_ret = SDL_GL_SetSwapInterval(config.vsync());

    // Если не получилось включить адаптивную вертикалку, то пробуем включить обычную
    if (vsync_ret < 0 && config.vsync() < 0)
    {
        Log::writef_debug("{} | vsync_ret < 0 && engine_params::vsync < 0 | {}", DV_FUNC_SIG, SDL_GetError());
        vsync_ret = SDL_GL_SetSwapInterval(-config.vsync());
    }

    // Вызывает ошибку при запуске через xvfb-run на сервере ГитХаба. Игнорируем
    if (vsync_ret < 0)
        Log::writef_error("{} | vsync_ret < 0 | {}", DV_FUNC_SIG, SDL_GetError());

    // Версия может отличаться от 30003 (например имеет значение 40002 при запуске
    // через Mesa на сервере ГитХаба)
    i32 gl_version = gladLoadGL(static_cast<GLADloadfunc>(SDL_GL_GetProcAddress));

    if (!gl_version)
    {
        Log::writef_error("{} | !gl_version", DV_FUNC_SIG);
        is_invalid_ = true;
        return;
    }

    Log::writef_info("GL_VENDOR: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    Log::writef_info("GL_RENDERER: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    Log::writef_info("GL_VERSION: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

OsWindow::~OsWindow()
{
    instance_ = nullptr;

    if (gl_context_)
        SDL_GL_DestroyContext(gl_context_);

    // Если удалить, то при завершении будет ошибка, которая не попадает в лог
    if (vk_device_)
        vkDestroyDevice(vk_device_, nullptr);

#if 0

    SDL_Vulkan_DestroySurface(vk_instance_, vk_surface_, nullptr);
    DestroyDebugUtilsMessengerEXT(vk_instance_, vk_debug_messenger_, nullptr);

    if (vk_instance_)
        vkDestroyInstance(vk_instance_, nullptr);
#endif

    if (window_)
        SDL_DestroyWindow(window_);
}

ivec2 OsWindow::get_size_in_pixels() const
{
    ivec2 ret;
    SDL_GetWindowSizeInPixels(window_, &ret.x, &ret.y);
    return ret;
}

} // namespace dviglo
