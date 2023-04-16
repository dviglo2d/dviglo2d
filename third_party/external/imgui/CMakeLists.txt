# Название таргета
set(target_name imgui)

# Создаём список файлов
file(GLOB source_files repo/*.cpp repo/src/*.h)
list(APPEND source_files repo/backends/imgui_impl_opengl3.cpp
                         repo/backends/imgui_impl_opengl3.h
                         repo/backends/imgui_impl_opengl3_loader.h
                         repo/backends/imgui_impl_sdl3.cpp
                         repo/backends/imgui_impl_sdl3.h
                         repo/misc/cpp/imgui_stdlib.cpp
                         repo/misc/cpp/imgui_stdlib.h
                         repo/misc/freetype/imgui_freetype.cpp
                         repo/misc/freetype/imgui_freetype.h)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Делаем заголовочные файлы доступными библиотеке и приложениям
target_include_directories(${target_name} PUBLIC repo repo/backends)

# Подключаем библиотеки
target_link_libraries(${target_name} PUBLIC SDL3-static freetype)

# Определяем дефайны здесь вместо изменения imconfig.h
target_compile_definitions(${target_name} PRIVATE IMGUI_ENABLE_FREETYPE=1)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/repo FILES ${source_files})
