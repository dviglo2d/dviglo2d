# Название таргета
set(target_name stb)

# Создаём список файлов
set(source_files repo/stb_image.h
                 repo/stb_image_write.h
                 repo/stb_rect_pack.h)

# Создаём интерфейсную библиотеку. Интерфейсная библиотека не компилируется,
# так как состоит только из включаемых файлов
add_library(${target_name} INTERFACE ${source_files})

# Делаем заголовочные файлы доступными таргетам, которые используют текущую библиотеку.
# SYSTEM подавляет предупреждения
target_include_directories(${target_name} SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/repo)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/repo FILES ${source_files})
