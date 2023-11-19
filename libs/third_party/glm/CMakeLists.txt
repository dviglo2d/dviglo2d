# Copyright (c) 2022-present the Dviglo project
# License: MIT

# Название таргета
set(target_name glm)

# Создаём список файлов. Файл glm.cpp игнорируем
file(GLOB_RECURSE source_files glm/*.h glm/*.hpp glm/*.inl)

# Создаём интерфейсную библиотеку. Интерфейсная библиотека не компилируется,
# так как состоит только из включаемых файлов
add_library(${target_name} INTERFACE ${source_files})

# Делаем заголовочные файлы доступными движку и приложениям
target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/glm FILES ${source_files})
