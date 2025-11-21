// Copyright (c) the Dviglo project
// License: MIT

#include "os_window.hpp"

#include <dv_log.hpp>
#include <dv_sdl_props.hpp>
#include <glad/gl.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

using namespace glm;
using namespace std;


namespace dviglo
{

// Список необходимых расширений
static vector<const char*> get_required_instance_extensions()
{
    u32 count;
    // В Windows возвращает [VK_KHR_SURFACE_EXTENSION_NAME ("VK_KHR_surface")
    // VK_KHR_WIN32_SURFACE_EXTENSION_NAME ("VK_KHR_win32_surface")
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);

    // Можно копировать указатели, так как строки в статической памяти
    vector<const char*> ret;
    for (u32 i = 0; i < count; ++i)
        ret.push_back(extensions[i]);

    return ret;
}

bool OsWindow::vk_create_instance()
{
    vector<const char*> extensions = get_required_instance_extensions();

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkResult vk_result = vkCreateInstance(&create_info, nullptr, &vk_instance_);
    if (vk_result != VK_SUCCESS)
    {
        Log::writef_error("{} | vkCreateInstance(...) | {}", DV_FUNC_SIG, string_VkResult(vk_result));
        is_invalid_ = true;
        return false;
    }

    return true;
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

OsWindow::~OsWindow()
{
    instance_ = nullptr;

    if (gl_context_)
        SDL_GL_DestroyContext(gl_context_);

    if (vk_instance_)
        vkDestroyInstance(vk_instance_, nullptr);

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
