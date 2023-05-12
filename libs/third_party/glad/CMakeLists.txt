# Copyright (c) 2022-2023 the Dviglo project
# License: MIT

# Только движок должен подключать этот таргет (приложениям нельзя)

# Название таргета
set(target_name glad)

# Создаём список файлов
file(GLOB_RECURSE source_files src/*.c include/*.h)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

if(DV_SHARED)
    # Макросы для библиотеки и движка
    target_compile_definitions(${target_name} PUBLIC GLAD_API_CALL_EXPORT GLAD_API_CALL_EXPORT_BUILD)
endif()

# Делаем заголовочные файлы доступными библиотеке и движку
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Нужно для компиляции динамической версии движка в GCC и Clang (добавляет -fPIC)
set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
