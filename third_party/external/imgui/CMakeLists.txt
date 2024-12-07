# Название таргета
set(target_name imgui)

# Создаём список файлов
file(GLOB source_files repo/*.cpp repo/src/*.h)
list(APPEND source_files repo/backends/imgui_impl_opengl3.cpp
                         repo/backends/imgui_impl_opengl3.h
                         repo/backends/imgui_impl_opengl3_loader.h
                         repo/backends/imgui_impl_sdl3.cpp
                         repo/backends/imgui_impl_sdl3.h)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Делаем заголовочные файлы доступными библиотеке и приложениям
target_include_directories(${target_name} PUBLIC repo repo/backends)

# Подключаем библиотеку
target_link_libraries(${target_name} PUBLIC SDL3-static)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/repo FILES ${source_files})
