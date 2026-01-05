// Copyright (c) the Dviglo project
// License: MIT

#include "os_window.hpp"

#include "../vk_utils/vk_utils.hpp"

#include <dv_log.hpp>
#include <dv_sdl_props.hpp>
#include <glad/gl.h>
#include <SDL3/SDL_vulkan.h>

#include <dv_file.hpp> // TODO: Убрать

using namespace glm;
using namespace std;


static_assert(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC);
// Хранилище для указателей на функции Vulkan
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace dviglo
{

static void vk_print_instance_extensions(const vector<vk::ExtensionProperties>& vk_instance_extensions)
{
    StrAscii vk_extensions_str;
    for (const vk::ExtensionProperties& ext : vk_instance_extensions)
    {
        if (!vk_extensions_str.empty())
            vk_extensions_str += ", ";

        vk_extensions_str += StrAscii(ext.extensionName.data()) + " (" + vk_version_to_string(ext.specVersion) + ")";
    }

    Log::writef_info("Vulkan instance extensions (with specVersion): {}", vk_extensions_str);
}

static void vk_print_instance_layers(const std::vector<vk::LayerProperties>& vk_layers)
{
    if (!vk_layers.size())
    {
        Log::write_info("No Vulkan instance layers available.");
        return;
    }

    Log::write_info("Vulkan instance layers (layerName | specVersion | implementationVersion | description):");

    for (const vk::LayerProperties& layer : vk_layers)
    {
        Log::writef_info("* {} | {} | {} | {}", layer.layerName.data(),
                                                vk_version_to_string(layer.specVersion),
                                                vk_version_to_string(layer.implementationVersion),
                                                layer.description.data());
    }
}

// Список обязательных расширений Vulkan.
// В Windows возвращает [VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME].
// Можно вызывать после SDL_InitSubSystem(SDL_INIT_VIDEO)
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

static VKAPI_ATTR vk::Bool32 VKAPI_CALL vk_debug_callback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT /*messageTypes*/,
                                                          vk::DebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                                          void* /*pUserData*/)
{
    const char* id_name = pCallbackData->pMessageIdName ? pCallbackData->pMessageIdName : "no_id_name";

    StrAscii object_names;

    for (u32 i = 0; i < pCallbackData->objectCount; ++i)
    {
        if (!object_names.empty())
            object_names += ", ";

        object_names += vk::to_string(pCallbackData->pObjects[i].objectType);

        if (pCallbackData->pObjects[i].pObjectName)
            object_names += StrAscii(" ") + pCallbackData->pObjects[i].pObjectName;
    }

    if (object_names.empty())
        object_names = "no_object_names";

    if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        Log::writef_error("VkDebug > {} > {}: {}", id_name, object_names, pCallbackData->pMessage);
    else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        Log::writef_warning("VkDebug > {} > {}: {}", id_name, object_names, pCallbackData->pMessage);
    else
        Log::writef_info("VkDebug > {} > {}: {}", id_name, object_names, pCallbackData->pMessage);

    return vk::False;
}

// Создаёт структуру vk::DebugUtilsMessengerCreateInfoEXT
static constexpr vk::DebugUtilsMessengerCreateInfoEXT create_debug_create_info()
{
    vk::DebugUtilsMessengerCreateInfoEXT ret {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = vk_debug_callback
    };

    return ret;
};

// Можно вызывать после SDL_InitSubSystem(SDL_INIT_VIDEO)
bool OsWindow::vk_create_instance()
{
    vector<const char*> extensions = get_required_instance_extensions();
    vector<const char*> layers;

#ifndef NDEBUG
    // Добавление этого выводит отладочные сообщения в консоль
    if (contains(vk_layers_, "VK_LAYER_KHRONOS_validation"))
        layers.push_back("VK_LAYER_KHRONOS_validation");

    // Добавление этого позволяет определить callback для вывода отладочных сообщений в файл
    if (contains(vk_instance_extensions_, vk::EXTDebugUtilsExtensionName))
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
#endif

    vk::ApplicationInfo app_info{ .apiVersion = VK_API_VERSION_1_0 };

    vk::InstanceCreateInfo create_info{ .pApplicationInfo = &app_info };
    create_info.setPEnabledLayerNames(layers)
               .setPEnabledExtensionNames(extensions);

#ifndef NDEBUG
    vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = create_debug_create_info();

    // Вывод отладочных сообщений для функций vkCreateInstance и vkDestroyInstance
    if (contains(vk_instance_extensions_, vk::EXTDebugUtilsExtensionName))
        create_info.setPNext(&debug_create_info);
#endif

    vk::Result vk_result;
    tie(vk_result, vk_instance_) = vk::createInstanceUnique(create_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk::createInstanceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    return true;
}

static void vk_print_physical_devices(const vector<vk::PhysicalDevice>& physical_devices)
{
    if (!physical_devices.size())
    {
        Log::write_info("No Vulkan physical devices available.");
        return;
    }

    Log::write_info("Vulkan physical devices:");

    for (const vk::PhysicalDevice& physical_device : physical_devices)
    {
        vk::PhysicalDeviceProperties properties = physical_device.getProperties();
        //vk::PhysicalDeviceFeatures features = physical_device.getFeatures();

        Log::writef_info("* {} ({}):", properties.deviceName.data(), vk::to_string(properties.deviceType));

        vector<vk::QueueFamilyProperties> queue_families = physical_device.getQueueFamilyProperties();

        if (!queue_families.size())
            Log::write_info("   No queue families available.");
        else
            Log::write_info("   Queue families:");

        for (const vk::QueueFamilyProperties& queue_family : queue_families)
            Log::writef_info("   * Count = {}, flags = {}", queue_family.queueCount, vk::to_string(queue_family.queueFlags));

        vk::PhysicalDeviceMemoryProperties memory_properties = physical_device.getMemoryProperties();

        if (!memory_properties.memoryHeapCount)
        {
            Log::write_info("   No memory heap available.");
        }
        else
        {
            Log::write_info("   Memory heaps:");

            for (u32 i = 0; i < memory_properties.memoryHeapCount; ++i)
            {
                vk::MemoryHeap& heap = memory_properties.memoryHeaps[i];

                Log::writef_info("   * Heap {}: size = {} B ({} GB, {} GiB), flags = {}",
                                 i,
                                 heap.size,
                                 heap.size / 1000 / 1000 / 1000,
                                 heap.size / 1024 / 1024 / 1024,
                                 vk::to_string(heap.flags));
            }

            Log::write_info("   Memory types:");

            for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i)
            {
                vk::MemoryType& type = memory_properties.memoryTypes[i];
                Log::writef_info("   * Type {}: heap_index = {}, flags = {}", i, type.heapIndex, vk::to_string(type.propertyFlags));
            }

            // В стандарте гарантируется не менее 128 байтов
            Log::writef_info("   Push constants limit = {} B", properties.limits.maxPushConstantsSize);
        }
    }
}

bool OsWindow::vk_create_device(const vk::PhysicalDevice& physical_device)
{
    float queue_priority = 0.f;

    vk::DeviceQueueCreateInfo queue_create_info {
        .queueFamilyIndex = 0,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    //vk::PhysicalDeviceFeatures features = physical_device.getFeatures();

    vk::DeviceCreateInfo device_create_info{
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        //.pEnabledFeatures = &features,
    };

    // Слои валидации для устройств являются deprecated даже в спецификации 1.0

    vk::Result vk_result;
    tie(vk_result, vk_device_) = physical_device.createDeviceUnique(device_create_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk::createDeviceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    // Так как используется только одно устройство, то для эффективности обновляем таблицу функций
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*vk_device_);

    return true;
}

OsWindow::OsWindow(const ConfigBase& config)
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    // Загружаем базовые функции: vkGetInstanceProcAddr, vkCreateInstance, vkEnumerateInstanceExtensionProperties,
    // vkEnumerateInstanceLayerProperties, vkEnumerateInstanceVersion (доступно в Vulkan 1.1)
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    vk::Result vk_result;

    // Доступные расширения для VkInstance
    tie(vk_result, vk_instance_extensions_) = vk::enumerateInstanceExtensionProperties();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk::enumerateInstanceExtensionProperties() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        is_invalid_ = true;
        return;
    }

    vk_print_instance_extensions(vk_instance_extensions_);

    // Доступные слои валидации
    tie(vk_result, vk_layers_) = vk::enumerateInstanceLayerProperties();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk::enumerateInstanceLayerProperties() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        is_invalid_ = true;
        return;
    }

    vk_print_instance_layers(vk_layers_);

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        Log::writef_error("{} | !SDL_InitSubSystem(SDL_INIT_VIDEO) | {}", DV_FUNC_SIG, SDL_GetError());
        is_invalid_ = true;
        return;
    }

    if (!vk_create_instance())
    {
        is_invalid_ = true;
        return;
    }

    // Загружаем все остальные функции
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*vk_instance_);

#ifndef NDEBUG
    // Вывод отладочных сообщений в лог
    if (contains(vk_instance_extensions_, vk::EXTDebugUtilsExtensionName))
    {
        vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = create_debug_create_info();
        tie(vk_result, vk_debug_utils_messenger_) = vk_instance_->createDebugUtilsMessengerEXTUnique(debug_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | createDebugUtilsMessengerEXTUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            is_invalid_ = true;
            return;
        }
    }
#endif // ndef NDEBUG

    // Доступные видеокарты
    vector<vk::PhysicalDevice> physical_devices;
    tie(vk_result, physical_devices) = vk_instance_->enumeratePhysicalDevices();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk::enumeratePhysicalDevices() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        is_invalid_ = true;
        return;
    }

    vk_print_physical_devices(physical_devices);

    if (!vk_create_device(physical_devices[0]))
    {
        is_invalid_ = true;
        return;
    }

    // TODO: Проверка лимита для Push Constants в функции выбора устройства

#if 1
    { // TODO: Просто тест
        fs::path base_path = get_base_path();
        vector<byte> shader_code = read_all_data(base_path / "engine_data/shaders/basic.vert.spv");

        vk::ShaderModuleCreateInfo sm_create_info
        {
            .codeSize = shader_code.size(),
            .pCode = reinterpret_cast<const uint32_t*>(shader_code.data()),
        };

        vk::UniqueShaderModule shader_module;
        tie(vk_result, shader_module) = vk_device_->createShaderModuleUnique(sm_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createShaderModuleUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            is_invalid_ = true;
            return;
        }
    }
#endif

    /*
    // TODO

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

    /*

        // TODO


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
        */

    is_invalid_ = true;
    return;
}

OsWindow::~OsWindow()
{
    instance_ = nullptr;

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
