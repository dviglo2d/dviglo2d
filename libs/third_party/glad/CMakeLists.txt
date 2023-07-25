# Copyright (c) 2022-2023 the Dviglo project
# License: MIT

# Только движок должен подключать этот таргет (приложениям нельзя)

# Название таргета
set(target_name glad)

# Создаём список файлов
file(GLOB_RECURSE source_files src/*.c include/*.h)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Делаем заголовочные файлы доступными библиотеке и движку
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
