Спецификация:
* 1.0:
  * [pdf](https://github.com/KhronosGroup/Vulkan-Web-Registry/blob/main/specs/1.0/pdf/vkspec.pdf)
  * [Quick Reference](https://github.com/KhronosGroup/Vulkan-Registry/blob/main/specs/1.0/refguide/Vulkan-1.0-web.pdf)
* Последняя версия:
  * [html (with all registered extensions)](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html)
  * [html (РКН заблокировал сайт)](https://docs.vulkan.org)
  * [pdf (with all registered extensions)](https://github.com/KhronosGroup/Vulkan-Registry/blob/main/specs/latest/pdf/vkspec.pdf)
  * [pdf (with all ratified extensions)](https://github.com/KhronosGroup/Vulkan-Registry/blob/main/specs/latest-ratified/pdf/vkspec.pdf)
* [Исходники документов](https://github.com/KhronosGroup/Vulkan-Docs)
* GLSL:
  * [GLSL для OpenGL](https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf)
  * [Отличия от GLSL для OpenGL](https://github.com/KhronosGroup/GLSL/blob/main/extensions/khr/GL_KHR_vulkan_glsl.txt)

Примеры: https://github.com/KhronosGroup/Vulkan-Samples/

Почему расширения VK_EXT_debug_report и VK_EXT_debug_marker были заменены на VK_EXT_debug_utils:
* https://www.lunarg.com/new-tutorial-for-vulkan-debug-utilities-extension
* https://github.com/KhronosGroup/Vulkan-Docs/blob/main/appendices/VK_EXT_debug_utils.adoc

Справка по `<vulkan/vulkan.hpp>`: https://github.com/KhronosGroup/Vulkan-Hpp

Справка по слоям валидации: https://github.com/KhronosGroup/Vulkan-ValidationLayers/tree/main/docs

--------------------------------

На данный момент в Vulkan всего 6 dispatchable хэндлов (VkInstance, VkPhysicalDevice, VkDevice, VkQueue,
VkCommandBuffer, VkExternalComputeQueueNV) - это всегда указатели и их нужно инициализировать nullptr.
Остальные хэндлы (non-dispatchable) - 64-битные числа и их нужно инициализировать VK_NULL_HANDLE
([источник](https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#fundamentals-validusage-handles))
