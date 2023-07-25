# Copyright (c) 2022-2023 the Dviglo project
# License: MIT

# Название таргета
set(target_name fmt)

# Создаём список файлов
file(GLOB_RECURSE source_files src/*.cc include/*.h)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Делаем заголовочные файлы доступными библиотеке и таргетам, которые используют библиотеку
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
