# Название таргета
set(target_name pugixml)

# Создаём список файлов
file(GLOB_RECURSE source_files repo/src/*.cpp repo/src/*.hpp)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Делаем заголовочные файлы доступными таргетам, которые используют текущую библиотеку
target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/repo/src)

# Определяем дефайны здесь вместо изменения pugiconfig.hpp
target_compile_definitions(${target_name} PUBLIC PUGIXML_NO_EXCEPTIONS)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/repo/src FILES ${source_files})
