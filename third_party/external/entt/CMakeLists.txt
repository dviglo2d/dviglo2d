# Название таргета
set(target_name entt)

# Создаём список файлов
file(GLOB_RECURSE source_files repo/src/*.h repo/src/*.hpp)

# Создаём интерфейсную библиотеку. Интерфейсная библиотека не компилируется,
# так как состоит только из включаемых файлов
add_library(${target_name} INTERFACE ${source_files})

# Делаем заголовочные файлы доступными таргетам, которые используют текущую библиотеку
target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/repo/src)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/repo/src FILES ${source_files})
