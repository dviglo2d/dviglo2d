# Название таргета
set(target_name dv_imgui)

# Создаём список файлов
file(GLOB source_files *.cpp *.hpp)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Делаем заголовочные файлы доступными приложениям
target_include_directories(${target_name} INTERFACE .)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})

# Подключаем библиотеки
target_link_libraries(${target_name} PUBLIC dviglo imgui)

# Выводим больше предупреждений
if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4)
else() # GCC, Clang или MinGW
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
endif()
