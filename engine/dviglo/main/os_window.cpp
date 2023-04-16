// Copyright (c) the Dviglo project
// License: MIT

#include "os_window.hpp"

#include "../vulkan/vulkan_utils.hpp"

#include <dv_log.hpp>
#include <dv_sdl_props.hpp>
#include <glad/gl.h>
#include <SDL3/SDL_vulkan.h>
#include <set>

#include "../res/image.hpp" // TODO: Убрать, это для скриншота

using namespace glm;
using namespace std;


namespace dviglo
{

// Печатает список доступных расширений Instance или Device
static void vk_print_extensions(const StrUtf8& prefix, const vector<vk::ExtensionProperties>& extensions, const StrUtf8& indent = "")
{
    StrAscii str;
    for (const vk::ExtensionProperties& ext : extensions)
    {
        if (!str.empty())
            str += ", ";

        str += StrAscii(ext.extensionName.data()) + " (" + vk_version_to_string(ext.specVersion) + ")";
    }

    if (str.empty())
        str = "none";

    Log::writef_info("{}{} extensions (with specVersion): {}", indent, prefix, str);
}

// Печатает список доступных слоёв валидации Instance или Device
static void vk_print_layers(const StrUtf8& prefix, const vector<vk::LayerProperties>& layers, const StrUtf8& indent = "")
{
    if (!layers.size())
    {
        Log::writef_info("{}{} layers: none", indent, prefix);
        return;
    }

    Log::writef_info("{}{} layers (layerName | specVersion | implementationVersion | description):", indent, prefix);

    for (const vk::LayerProperties& layer : layers)
    {
        Log::writef_info("{}* {} | {} | {} | {}", indent,
                                                  layer.layerName.data(),
                                                  vk_version_to_string(layer.specVersion),
                                                  vk_version_to_string(layer.implementationVersion),
                                                  layer.description.data());
    }
}

#ifndef NDEBUG
// Колбэк для вывода сообщений от слоёв валидации
static VKAPI_ATTR vk::Bool32 VKAPI_CALL vk_debug_callback(vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT /*message_types*/,
                                                          vk::DebugUtilsMessengerCallbackDataEXT const* callback_data,
                                                          void* /*user_data*/)
{
    const char* id_name = callback_data->pMessageIdName ? callback_data->pMessageIdName : "no_id_name";

    StrAscii object_names;
    for (u32 i = 0; i < callback_data->objectCount; ++i)
    {
        if (!object_names.empty())
            object_names += ", ";

        object_names += vk::to_string(callback_data->pObjects[i].objectType);

        if (callback_data->pObjects[i].pObjectName)
            object_names += StrAscii(" ") + callback_data->pObjects[i].pObjectName;
    }

    if (object_names.empty())
        object_names = "no_object_names";

    if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        Log::writef_error("VkDebug > {} > {}: {}", id_name, object_names, callback_data->pMessage);
    else if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        Log::writef_warning("VkDebug > {} > {}: {}", id_name, object_names, callback_data->pMessage);
    else
        Log::writef_info("VkDebug > {} > {}: {}", id_name, object_names, callback_data->pMessage);

    return vk::False;
}

// Создаёт и заполняет структуру vk::DebugUtilsMessengerCreateInfoEXT
static constexpr vk::DebugUtilsMessengerCreateInfoEXT create_debug_info()
{
    vk::DebugUtilsMessengerCreateInfoEXT ret
    {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = vk_debug_callback,
    };

    return ret;
};
#endif

// Список обязательных расширений Vulkan.
// В Windows возвращает [VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME].
// Можно вызывать после SDL_InitSubSystem(SDL_INIT_VIDEO),
// но в документации написано, что нужно вызывать после создания окна с флагом SDL_WINDOW_VULKAN
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
    assert(window_); // Читайте описание get_required_instance_extensions()

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

    // https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_get_physical_device_properties2.html
    // Требуется для VK_EXT_descriptor_indexing:
    // https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_indexing.html
    extensions.push_back(vk::KHRGetPhysicalDeviceProperties2ExtensionName);

    vk::ApplicationInfo app_info { .apiVersion = VK_API_VERSION_1_0 };

    vk::InstanceCreateInfo instance_info { .pApplicationInfo = &app_info };
    instance_info.setPEnabledLayerNames(layers)
                 .setPEnabledExtensionNames(extensions);

#ifndef NDEBUG
    vk::DebugUtilsMessengerCreateInfoEXT debug_info = create_debug_info();

    // Вывод отладочных сообщений для функций vkCreateInstance и vkDestroyInstance
    if (contains(vk_instance_extensions_, vk::EXTDebugUtilsExtensionName))
        instance_info.setPNext(&debug_info);
#endif

    vk::Result vk_result;
    tie(vk_result, vk_instance_) = vk::createInstanceUnique(instance_info).asTuple();

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
        }

        // В стандарте гарантируется не менее 128 байтов
        Log::writef_info("   Push constants limit = {} B", properties.limits.maxPushConstantsSize);

        // Печатаем доступные расширений для устройства
        {
            auto [vk_result, extensions] = physical_device.enumerateDeviceExtensionProperties();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | physical_device.enumerateDeviceExtensionProperties() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                continue;
            }

            vk_print_extensions("Vulkan device", extensions, "   ");
        }

        // Печатаем доступные слои валидации для устройства (их использовать нельзя, они deprecated)
        {
            auto [vk_result, layers] = physical_device.enumerateDeviceLayerProperties();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | physical_device.enumerateDeviceLayerProperties() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                continue;
            }

            vk_print_layers("Vulkan device", layers, "   ");
        }
    }
}

static void transition_layout(vk::CommandBuffer command_buffer, vk::Image image,
                              vk::PipelineStageFlags source_stage, vk::PipelineStageFlags destination_stage,
                              vk::AccessFlags src_access_mask, vk::AccessFlags dst_access_mask,
                              vk::ImageLayout old_layout, vk::ImageLayout new_layout)
{
    vk::ImageMemoryBarrier barrier
    {
        .srcAccessMask = src_access_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = image,
        .subresourceRange =
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    command_buffer.pipelineBarrier(source_stage, destination_stage,
                                   {},
                                   {},
                                   {},
                                   barrier);
}

static bool save_screenshot(vk::Device device, vk::PhysicalDevice physical_device, vk::Queue queue,
                            vk::CommandPool command_pool, vk::Image src_image, u32 width, u32 height,
                            vma::Allocator vma_allocator)
{
    vk::ImageCreateInfo image_info
    {
        .imageType = vk::ImageType::e2D,
        .format = vk::Format::eR8G8B8A8Unorm,
        .extent = {width, height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eLinear,
        .usage = vk::ImageUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    vma::AllocationCreateInfo vma_allocation_info
    {
        .flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
        .usage = vma::MemoryUsage::eAuto,
        .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    };

    vk::Result vk_result;

    VmaAllocatedImage vma_allocated_image;
    tie(vk_result, vma_allocated_image) = vma_allocator.createImageUnique(image_info, vma_allocation_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vma_allocator_->createImageUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    // -------------------------------- Выделяем командный буфер из пула

    vk::CommandBufferAllocateInfo cb_alloc_info
    {
        .commandPool = command_pool,
        .commandBufferCount = 1,
    };

    vector<vk::UniqueCommandBuffer> command_buffers;
    tie(vk_result, command_buffers) = device.allocateCommandBuffersUnique(cb_alloc_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk_device_->allocateCommandBuffersUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    vk::CommandBuffer command_buffer = *(command_buffers[0]);

    // -------------------------------- Пишем в коммандный буфер

    vk::CommandBufferBeginInfo command_buffer_begin_info
    {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };

    vk_result = command_buffer.begin(command_buffer_begin_info);

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | command_buffer->begin(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    transition_layout(command_buffer, *vma_allocated_image.second, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    transition_layout(command_buffer, src_image, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eTransferRead, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferSrcOptimal);

    // use image copy (requires us to manually flip components)
    vk::ImageCopy image_copy_region
    {
        .srcSubresource
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .layerCount = 1,
        },
        .dstSubresource
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .layerCount = 1,
        },
        .extent = {width, height, 1},
    };

    command_buffer.copyImage(src_image, vk::ImageLayout::eTransferSrcOptimal,
                             *vma_allocated_image.second, vk::ImageLayout::eTransferDstOptimal,
                             image_copy_region);

    vk_result = command_buffer.end();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | command_buffer.end() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    // -------------------------------- Забор

    vk::UniqueFence fence;

    vk::FenceCreateInfo fence_info;

    tie(vk_result, fence) = device.createFenceUnique(fence_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk_device_->createFenceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    // --------------------------------Отправляем в очередь

    vk::SubmitInfo submit_info
    {
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    vk_result = queue.submit(1, &submit_info, *fence);

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | compute_queue.submit(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    // -------------------------------- Ждем результат

    vk_result = device.waitForFences(1, &fence.get(), vk::True, 100000000000);

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk_device_->waitForFences(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    // -------------------------------- Сохраняем результат

    vk::ImageSubresource subresource
    {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
    };

    vk::SubresourceLayout subresource_layout = device.getImageSubresourceLayout(*vma_allocated_image.second, subresource);

    void* data;
    tie(vk_result, data) = vma_allocator.mapMemory(*vma_allocated_image.first);

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vma_allocator.mapMemory(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    Image img(512, 512, 4, 0x00000000F);

    const uint8_t* src_data = reinterpret_cast<const uint8_t*>(data);
    u8* dst_data = img.pixel_ptr(0, 0);

    for (u32 y = 0; y < height; ++y)
    {
        memcpy(dst_data + (y * width * 4),
               src_data + (y * subresource_layout.rowPitch),
               width * 4);
    }

    //img.save_png("f:/f_temp/22/111.png");
    img.save_png("111.png");

    vma_allocator.unmapMemory(*vma_allocated_image.first);
    // Чтобы вручную не маппить/анмаппить можно добавить флаг vma_allocation_info.flags ~ vma::AllocationCreateFlagBits::eMapped

    return true;
}


bool OsWindow::vk_pick_physical_device(const vector<vk::PhysicalDevice>& physical_devices)
{
    vk_graphics_queue_family_index_ = 0;
    vk_present_queue_family_index_ = 0;
    vk_physical_device_ = physical_devices[0];

    // TODO: Проверить наличие расширения VK_KHR_SWAPCHAIN_EXTENSION_NAME и KHRMaintenance3ExtensionName

    if (!vk_physical_device_)
    {
        Log::writef_error("{} | Failed to find a suitable GPU!", DV_FUNC_SIG);
        return false;
    }
    else
    {
        return true;
    }
}

bool OsWindow::vk_create_device()
{
    assert(vk_physical_device_);
    assert(vk_graphics_queue_family_index_ != numeric_limits<u32>::max() && vk_present_queue_family_index_ != numeric_limits<u32>::max());

    // На случай, если эти два семейства очередей - одно семейство
    set<u32> unique_queue_family_indices {vk_graphics_queue_family_index_, vk_present_queue_family_index_};

    float queue_priority = 1.f;
    vector<vk::DeviceQueueCreateInfo> queue_infos;

    for (u32 family_index : unique_queue_family_indices)
    {
        vk::DeviceQueueCreateInfo queue_info
        {
            .queueFamilyIndex = family_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        queue_infos.push_back(queue_info);
    }

    vector<const char*> extensions
    {
        // https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_swapchain.html
        vk::KHRSwapchainExtensionName,

        // https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_indexing.html
        vk::EXTDescriptorIndexingExtensionName,

        // https://docs.vulkan.org/refpages/latest/refpages/source/VK_KHR_maintenance3.html
        // Требуется для VK_EXT_descriptor_indexing:
        // https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_indexing.html
        vk::KHRMaintenance3ExtensionName
    };

    vk::PhysicalDeviceFeatures features = vk_physical_device_.getFeatures();

    // https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_indexing.html
    // https://docs.vulkan.org/refpages/latest/refpages/source/VkPhysicalDeviceDescriptorIndexingFeatures.html
    vk::PhysicalDeviceDescriptorIndexingFeaturesEXT indexing_features
    {
        // Разрешаем частично заполненные массивы
        .descriptorBindingPartiallyBound = vk::True,

        // Разрешаем безразмерные массивы в шейдере:
        // https://docs.vulkan.org/guide/latest/descriptor_arrays.html
        .runtimeDescriptorArray = vk::True,
    };

    vk::DeviceCreateInfo device_info
    {
        // Требуется для VK_EXT_descriptor_indexing:
        // https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_indexing.html
        .pNext = &indexing_features,

        .pEnabledFeatures = &features, // TODO: Включать только то, что нужно
    };

    device_info.setQueueCreateInfos(queue_infos)
               .setPEnabledExtensionNames(extensions);

    // Слои валидации для устройств являются deprecated даже в спецификации 1.0

    vk::Result vk_result;
    tie(vk_result, vk_device_) = vk_physical_device_.createDeviceUnique(device_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | vk_physical_device_.createDeviceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return false;
    }

    // Так как используется только одно устройство, то для эффективности обновляем таблицу функций
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*vk_device_);

    // Эти две очереди скорее всего будут одной очередью
    vk_graphics_queue_ = vk_device_->getQueue(vk_graphics_queue_family_index_, 0);
    vk_present_queue_ = vk_device_->getQueue(vk_present_queue_family_index_, 0);

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

    // Печатаем доступные расширения для VkInstance
    {
        tie(vk_result, vk_instance_extensions_) = vk::enumerateInstanceExtensionProperties();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk::enumerateInstanceExtensionProperties() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        vk_print_extensions("Vulkan instance", vk_instance_extensions_);
    }

    // Печатаем доступные слои валидации
    {
        tie(vk_result, vk_layers_) = vk::enumerateInstanceLayerProperties();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk::enumerateInstanceLayerProperties() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        vk_print_layers("Vulkan instance", vk_layers_);
    }

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        Log::writef_error("{} | !SDL_InitSubSystem(SDL_INIT_VIDEO) | {}", DV_FUNC_SIG, SDL_GetError());
        return;
    }

    // Создаём окно
    {
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
            return;
        }
    }

    if (!vk_create_instance())
        return;

    // Загружаем все остальные функции
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*vk_instance_);

#ifndef NDEBUG
    // Вывод отладочных сообщений в лог
    if (contains(vk_instance_extensions_, vk::EXTDebugUtilsExtensionName))
    {
        vk::DebugUtilsMessengerCreateInfoEXT debug_info = create_debug_info();
        tie(vk_result, vk_debug_utils_messenger_) = vk_instance_->createDebugUtilsMessengerEXTUnique(debug_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_instance_->createDebugUtilsMessengerEXTUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }
    }
#endif // ndef NDEBUG

    // Создаём поверхность перед выбором физического устройства, так как поверхность может повлиять на выбор
    {
        VkSurfaceKHR raw_vk_surface = VK_NULL_HANDLE;

        if (!SDL_Vulkan_CreateSurface(window_, *vk_instance_, nullptr, &raw_vk_surface))
        {
            Log::writef_error("{} | !SDL_Vulkan_CreateSurface(...) | {}", DV_FUNC_SIG, SDL_GetError());
            return;
        }

        vk_surface_ = vk::UniqueSurfaceKHR(raw_vk_surface, *vk_instance_);
    }

    // Выбираем физическое устройство
    {
        vector<vk::PhysicalDevice> physical_devices;
        tie(vk_result, physical_devices) = vk_instance_->enumeratePhysicalDevices();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_instance_->enumeratePhysicalDevices() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        vk_print_physical_devices(physical_devices);

        if (!vk_pick_physical_device(physical_devices))
            return;
    }

    if (!vk_create_device())
        return;

    // Аллокатор памяти
    {
        vma::VulkanFunctions vma_functions = vma::functionsFromDispatcher();

        vma::AllocatorCreateInfo vma_allocator_info
        {
            .physicalDevice = vk_physical_device_,
            .device = *vk_device_,
            .pVulkanFunctions = &vma_functions,
            .instance = *vk_instance_,
        };

        tie(vk_result, vma_allocator_) = vma::createAllocatorUnique(vma_allocator_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vma::createAllocatorUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }
    }

    // Командный пул
    {
        vk::CommandPoolCreateInfo command_pool_info
        {
            .queueFamilyIndex = 0, // TODO: Индекс может быть другим
        };

        tie(vk_result, vk_command_pool_) = vk_device_->createCommandPoolUnique(command_pool_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createCommandPoolUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }
    }

    // TODO: Проверка лимита для Push Constants в функции выбора устройства



    optional<Swapchain> swaphain = Swapchain::construct(vk_physical_device_, *vk_device_,
                                                        vk_graphics_queue_family_index_, vk_present_queue_family_index_,
                                                        *vk_surface_,
                                                        get_size_in_pixels(),
                                                        /*get_size_in_pixels(),//*/ivec2(500, 612), // Размер offscreen буфера может отличатсья
                                                        vma_allocator_.get(),
                                                        vk::PresentModeKHR::eImmediate);
                                                        //vk::PresentModeKHR::eMailbox);
                                                        //vk::PresentModeKHR::eFifo);

    if (!swaphain)
    {
        Log::writef_error("{} | !swaphain.is_valid()", DV_FUNC_SIG);
        return;
    }

    swapchain_ = make_unique<Swapchain>(std::move(*swaphain));

#if 1
    { // TODO: Тест вычислительного шейдера

        // -------------------------------- Загружаем шейдер из файла

        fs::path base_path = get_base_path();
        vk::UniqueShaderModule shader_module = load_shader(*vk_device_, base_path / "engine_data/shaders/test.comp.spv");

        if (!shader_module)
        {
            Log::writef_error("{} | !shader_module", DV_FUNC_SIG);
            return;
        }

        // -------------------------------- Создаём буфер и выделяем память

        vk::BufferCreateInfo vk_buffer_info
        {
            .size = 1024 * sizeof(f32),
            .usage = vk::BufferUsageFlagBits::eStorageBuffer,
            //.sharingMode = vk::SharingMode::eExclusive
        };

        vma::AllocationCreateInfo vma_allocation_info
        {
            .flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
            .usage = vma::MemoryUsage::eAuto,
        };

        VmaAllocatedBuffer vma_allocated_buffer;
        tie(vk_result, vma_allocated_buffer) = vma_allocator_->createBufferUnique(vk_buffer_info, vma_allocation_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vma_allocator_->createBufferUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Заполняем буфер данными

        std::vector<float> test_data(1024);
        for (int i = 0; i < 1024; i++)
            test_data[i] = static_cast<float>(i);

        void* data;
        tie(vk_result, data) = vma_allocator_->mapMemory(*vma_allocated_buffer.first);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vma_allocator_->mapMemory(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        memcpy((char*)data, test_data.data(), test_data.size() * sizeof(float));

        vma_allocator_->unmapMemory(*vma_allocated_buffer.first);

        // -------------------------------- Макет наборов дескрипторов

        vk::DescriptorBindingFlagsEXT bind_flag = vk::DescriptorBindingFlagBitsEXT::ePartiallyBound;

        vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info
        {
            .bindingCount = 1,
            .pBindingFlags = &bind_flag,
        };

        vk::DescriptorSetLayoutBinding layout_binding
        {
            .binding = 0,
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            //.descriptorCount = 1,
            .descriptorCount = 1000, // ДЕЛАЕМ МАССИВ НА 1000 БУФЕРОВ
            .stageFlags = vk::ShaderStageFlagBits::eCompute,
        };

        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info
        {
            .pNext = &extended_info, // ЦЕПЛЯЕМ ФЛАГИ
            .bindingCount = 1,
            .pBindings = &layout_binding,
        };

        vk::UniqueDescriptorSetLayout vk_descriptor_set_layout;

        tie(vk_result, vk_descriptor_set_layout) = vk_device_->createDescriptorSetLayoutUnique(descriptor_set_layout_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createDescriptorSetLayoutUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Макет пайплпайна

        // Описываем, что мы будем передавать один uint32_t (размер 4 байта)
        vk::PushConstantRange push_constant_range
        {
            .stageFlags = vk::ShaderStageFlagBits::eCompute,
            .offset = 0,
            .size = sizeof(u32),
        };

        vk::PipelineLayoutCreateInfo pipeline_layout_info
        {
            .setLayoutCount = 1,
            .pSetLayouts = &(*vk_descriptor_set_layout),

            // Push константы
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &push_constant_range,
        };

        vk::UniquePipelineLayout vk_pipeline_layout;

        tie(vk_result, vk_pipeline_layout) = vk_device_->createPipelineLayoutUnique(pipeline_layout_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createPipelineLayoutUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Пайплайн

        vk::PipelineShaderStageCreateInfo pipeline_shader_stage_info
        {
            .stage = vk::ShaderStageFlagBits::eCompute,
            .module = *shader_module,
            .pName = "main",
        };

        vk::ComputePipelineCreateInfo compute_pipeline_info
        {
            .stage = pipeline_shader_stage_info,
            .layout = *vk_pipeline_layout,
        };

        vk::UniquePipeline vk_pipeline;

        tie(vk_result, vk_pipeline) = vk_device_->createComputePipelineUnique(nullptr, compute_pipeline_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createComputePipelineUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Выделяем командный буфер из пула

        vk::CommandBufferAllocateInfo command_buffer_allocate_info
        {
            .commandPool = *vk_command_pool_,
            .commandBufferCount = 1,
        };

        vector<vk::UniqueCommandBuffer> command_buffers;
        tie(vk_result, command_buffers) = vk_device_->allocateCommandBuffersUnique(command_buffer_allocate_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->allocateCommandBuffersUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Создаём пул дескрипторов

        vk::DescriptorPoolSize pool_size
        {
            .type = vk::DescriptorType::eStorageBuffer,
            //.descriptorCount = 1,
            .descriptorCount = 1000,
        };


        vk::DescriptorPoolCreateInfo descriptor_pool_info
        {
            .maxSets = 10,
            .poolSizeCount = 1,
            .pPoolSizes = &pool_size,
        };

        vk::UniqueDescriptorPool vk_unique_descriptor_pool;

        tie(vk_result, vk_unique_descriptor_pool) = vk_device_->createDescriptorPoolUnique(descriptor_pool_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createDescriptorPoolUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Выделяем набор дескрипторов из пула

        vk::DescriptorSetAllocateInfo descriptor_set_allocate_info
        {
            .descriptorPool = *vk_unique_descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &(*vk_descriptor_set_layout),
        };

        vector<vk::DescriptorSet> vk_descriptor_sets;

        tie(vk_result, vk_descriptor_sets) = vk_device_->allocateDescriptorSets(descriptor_set_allocate_info);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->allocateDescriptorSets(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Пишем конкретные данные в дескриптор

        vk::DescriptorSet descriptor_set = vk_descriptor_sets[0];

        vk::DescriptorBufferInfo descriptor_buffer_info
        {
            .buffer = *vma_allocated_buffer.second,
            .range = 1024 * sizeof(float),
        };

        // Кладём буфер в ячейку 0
        uint32_t target_buffer_index = 0;

        vk::WriteDescriptorSet write_descriptor_set
        {
            .dstSet = descriptor_set,
            .dstBinding = 0,
            .dstArrayElement = target_buffer_index, // ИНДЕКС В МАССИВЕ
            .descriptorCount = 1, // ПИШЕМ ТОЛЬКО 1 БУФЕР
            .descriptorType = vk::DescriptorType::eStorageBuffer,
            .pBufferInfo = &descriptor_buffer_info,
        };

        vk_device_->updateDescriptorSets(1, &write_descriptor_set, 0, nullptr);

        // -------------------------------- Пишем в коммандный буфер

        vk::CommandBuffer command_buffer = *(command_buffers[0]);

        vk::CommandBufferBeginInfo command_buffer_begin_info;

        vk_result = command_buffer.begin(command_buffer_begin_info);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | command_buffer->begin(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, *vk_pipeline);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *vk_pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

        // ПЕРЕДАЕМ ИНДЕКС В ШЕЙДЕР
        command_buffer.pushConstants<uint32_t>(*vk_pipeline_layout, vk::ShaderStageFlagBits::eCompute, 0, target_buffer_index);

        command_buffer.dispatch(1, 1, 1);

        vk_result = command_buffer.end();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | command_buffer.end() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Забор

        vk::UniqueFence fence;

        vk::FenceCreateInfo fence_info;

        tie(vk_result, fence) = vk_device_->createFenceUnique(fence_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createFenceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Отправляем в очередь

        vk::SubmitInfo submit_info
        {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
        };

        vk_result = vk_graphics_queue_.submit(1, &submit_info, *fence);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | compute_queue.submit(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Ждем результат

        vk_result = vk_device_->waitForFences(1, &fence.get(), vk::True, 100000000000);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->waitForFences(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Читаем данные назад

        tie(vk_result, data) = vma_allocator_->mapMemory(*vma_allocated_buffer.first);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vma_allocator_->mapMemory(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        std::vector<float> out_data(1024);
        std::memcpy(out_data.data(), data, 1024 * sizeof(float));

        vma_allocator_->unmapMemory(*vma_allocated_buffer.first);

        // -------------------------------- Проверяем, что получили правильный результат

        bool is_correct = true;

        for (i32 i = 0; i < 1024; i++)
        {
            f32 input = static_cast<f32>(i);
            f32 expected = input + 1.0f;
            f32 actual = out_data[i];

            if (std::abs(actual - expected) > 0.0001f)
            {
                is_correct = false;
                break;
            }
        }

        if (!is_correct)
            Log::writef_error("{} | !!!!!!!!! Compute shader: Incorrect result!", DV_FUNC_SIG);
        else
            Log::writef_info("{} | !!!!!!!!! Compute shader: Correct result!", DV_FUNC_SIG);
    }
#endif

#if 0
    {
        // TODO: Тест рендеринга треугольника

        // -------------------------------- Создаём изображение

        vk::ImageCreateInfo vk_image_info
        {
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8G8B8A8Unorm,
            .extent = { 512, 512, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined,
        };

        vma::AllocationCreateInfo vma_allocation_info
        {
            .usage = vma::MemoryUsage::eAuto,
        };

        VmaAllocatedImage vma_allocated_image;
        tie(vk_result, vma_allocated_image) = vma_allocator_->createImageUnique(vk_image_info, vma_allocation_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vma_allocator_->createImageUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Вьюер изображения

        vk::ImageViewCreateInfo vk_image_view_create_info
        {
            .image = *vma_allocated_image.second,
            .viewType = vk::ImageViewType::e2D,
            .format = vk::Format::eR8G8B8A8Unorm,
            .subresourceRange =
            {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vk::UniqueImageView vk_image_view;
        tie(vk_result, vk_image_view) = vk_device_->createImageViewUnique(vk_image_view_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createImageViewUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Проход рендеринга

        vk::AttachmentDescription color_attachment
        {
            .format = vk::Format::eR8G8B8A8Unorm,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,

            // Так нельзя, потому что для VK_IMAGE_LAYOUT_PRESENT_SRC_KHR требуется VK_KHR_swapchain, которого у нас нет.
            // При копировании в память CPU надо использовать VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
            .finalLayout = vk::ImageLayout::ePresentSrcKHR,
        };

        vk::AttachmentReference color_attachment_ref
        {
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal,
        };
        

        vk::SubpassDescription subpass
        {
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref,
        };

        vk::RenderPassCreateInfo render_pass_create_info
        {
            .attachmentCount = 1,
            .pAttachments = &color_attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
        };

        vk::UniqueRenderPass vk_render_pass;
        tie(vk_result, vk_render_pass) = vk_device_->createRenderPassUnique(render_pass_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createRenderPassUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Фреймбуфер

        vk::FramebufferCreateInfo vk_framebuffer_create_info
        {
            .renderPass = *vk_render_pass,
            .attachmentCount = 1,
            .pAttachments = &vk_image_view.get(),
            .width = 512,
            .height = 512,
            .layers = 1,
        };

        vk::UniqueFramebuffer vk_framebuffer;
        tie(vk_result, vk_framebuffer) = vk_device_->createFramebufferUnique(vk_framebuffer_create_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createFramebufferUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Загружаем шейдеры из файлов

        fs::path base_path = get_base_path();
        vk::UniqueShaderModule vert_shader_module = load_shader(*vk_device_, base_path / "engine_data/shaders/simple.vert.spv");

        if (!vert_shader_module)
        {
            Log::writef_error("{} | !vert_shader_module", DV_FUNC_SIG);
            return;
        }

        vk::UniqueShaderModule frag_shader_module = load_shader(*vk_device_, base_path / "engine_data/shaders/simple.frag.spv");

        if (!frag_shader_module)
        {
            Log::writef_error("{} | !frag_shader_module", DV_FUNC_SIG);
            return;
        }

        // -------------------------------- Графический конвеер

        // -------------------------------- Шейдерные стадии

        vk::PipelineShaderStageCreateInfo pipeline_vert_shader_stage_create_info
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *vert_shader_module,
            .pName = "main",
        };

        vk::PipelineShaderStageCreateInfo pipeline_frag_shader_stage_create_info
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *frag_shader_module,
            .pName = "main",
        };

        vk::PipelineShaderStageCreateInfo shader_stages[] = { pipeline_vert_shader_stage_create_info, pipeline_frag_shader_stage_create_info };

        // -------------------------------- Стадия сборки примитивов из вершин

        vk::PipelineVertexInputStateCreateInfo vertex_input_info; // Все по дефолту и обнулено

        // -------------------------------- Стадия сборки примитивов из вершин

        vk::PipelineInputAssemblyStateCreateInfo assemply_info
        {
            .topology = vk::PrimitiveTopology::eTriangleList,
        };

        // -------------------------------- Вьюпорт

        vk::Viewport viewport
        {
            .x = 0.f,
            .y = 0.f,
            .width = 512.f,
            .height = 512.f,
            .minDepth = 0.f,
            .maxDepth = 1.f,
        };

        vk::Rect2D scissor
        {
            .offset = {0, 0},
            .extent = {512, 512},
        };

        vk::PipelineViewportStateCreateInfo viewport_state
        {
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
        };

        // -------------------------------- Растеризатор

        vk::PipelineRasterizationStateCreateInfo rasterizer
        {
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eClockwise,
            .lineWidth = 1.f,
        };

        // -------------------------------- Мультисэмплинг

        vk::PipelineMultisampleStateCreateInfo multisampling
        {
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .minSampleShading = 1.f,
        };

        // -------------------------------- Смешивание цветов в цветовом включении

        vk::PipelineColorBlendAttachmentState color_blend_attachment
        {
            .blendEnable = false,
            .srcColorBlendFactor = vk::BlendFactor::eOne,
            .dstColorBlendFactor = vk::BlendFactor::eZero,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
        };

        vk::PipelineColorBlendStateCreateInfo color_blending
        {
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
        };

        // -------------------------------- Макет пайплана

        vk::PipelineLayoutCreateInfo pipeline_layout_info; // Всё пусто

        vk::UniquePipelineLayout vk_graphics_pipeline_layout;
        tie(vk_result, vk_graphics_pipeline_layout) = vk_device_->createPipelineLayoutUnique(pipeline_layout_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createPipelineLayoutUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Пайплайн

        vk::GraphicsPipelineCreateInfo graphics_pipeline_info
        {
            .stageCount = 2,
            .pStages = shader_stages,
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &assemply_info,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &color_blending,
            .layout = *vk_graphics_pipeline_layout,
            .renderPass = *vk_render_pass,
        };

        vk::UniquePipeline vk_graphics_pipeline;
        tie(vk_result, vk_graphics_pipeline) = vk_device_->createGraphicsPipelineUnique(nullptr, graphics_pipeline_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createGraphicsPipelineUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Рендерим

        // -------------------------------- Выделяем командный буфер из пула

        vk::CommandBufferAllocateInfo cb_alloc_info
        {
            .commandPool = *vk_command_pool_,
            .commandBufferCount = 1,
        };

        vector<vk::UniqueCommandBuffer> command_buffers;
        tie(vk_result, command_buffers) = vk_device_->allocateCommandBuffersUnique(cb_alloc_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->allocateCommandBuffersUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        vk::CommandBuffer command_buffer = *(command_buffers[0]);

        // -------------------------------- Пишем в коммандный буфер

        vk::CommandBufferBeginInfo command_buffer_begin_info;

        vk_result = command_buffer.begin(command_buffer_begin_info);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | command_buffer->begin(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        vk::ClearValue clear_color
        {
            .color
            {
                .float32 = {{0.0f, 0.0f, 0.0f, 1.0f}}
            }
        };

        vk::RenderPassBeginInfo rp_info
        {
            .renderPass = *vk_render_pass,
            .framebuffer = *vk_framebuffer,
            .renderArea = {.offset = {0, 0}, .extent = {512, 512}},
            .clearValueCount = 1,
            .pClearValues = &clear_color,
        };

        command_buffer.beginRenderPass(rp_info, vk::SubpassContents::eInline);
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *vk_graphics_pipeline);
        command_buffer.draw(3, 1, 0, 0);
        command_buffer.endRenderPass();

        vk_result = command_buffer.end();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | command_buffer.end() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Забор

        vk::UniqueFence fence;

        vk::FenceCreateInfo fence_info;

        tie(vk_result, fence) = vk_device_->createFenceUnique(fence_info).asTuple();

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->createFenceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Отправляем в очередь

        vk::SubmitInfo submit_info
        {
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
        };

        vk_result = vk_graphics_queue_.submit(1, &submit_info, *fence);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | compute_queue.submit(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        // -------------------------------- Ждем результат

        vk_result = vk_device_->waitForFences(1, &fence.get(), vk::True, 100000000000);

        if (vk_result != vk::Result::eSuccess)
        {
            Log::writef_error("{} | vk_device_->waitForFences(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
            return;
        }

        if (!save_screenshot(*vk_device_, vk_physical_device_, vk_graphics_queue_, *vk_command_pool_, *vma_allocated_image.second, 512, 512, *vma_allocator_))
            return;
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

    //is_valid_ = false;
    is_valid_ = true;
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
