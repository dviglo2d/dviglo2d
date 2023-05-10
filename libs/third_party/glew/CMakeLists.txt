# Copyright (c) 2022-2023 the Dviglo project
# License: MIT

# Название таргета
set(target_name glew)

# Создаём список файлов
file(GLOB_RECURSE source_files src/*.c include/*.h)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Дефайны для библиотеки и для таргетов, которые используют библиотеку
target_compile_definitions(${target_name} PUBLIC GLEW_STATIC GLEW_NO_GLU)

# Делаем заголовочные файлы доступными библиотеке и таргетам, которые используют библиотеку
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Нужно для компиляции динамической версии движка в GCC и Clang (добавляет -fPIC)
set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Подключаем OpenGL к таргетам, которые используют текущую библиотеку
find_package(OpenGL)
target_link_libraries(${target_name} INTERFACE ${OPENGL_LIBRARIES})
target_include_directories(${target_name} INTERFACE ${OPENGL_INCLUDE_DIR})

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
