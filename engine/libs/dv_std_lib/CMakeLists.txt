# Указываем минимальную версию CMake
cmake_minimum_required(VERSION 3.16)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(root_file TRUE)
else()
    set(root_file FALSE)
endif()

# Название проекта
project(dv_std_lib)

option(DV_STD_LIB_TESTER "Компилировать Тестер" OFF)
option(DV_STD_LIB_UB_SANITIZER "Детектировать undefined behavior" OFF)

# Версия стандарта C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Если используется одноконфигурационный генератор и конфигурация не указана
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    # то конфигурацией по умолчанию будет Release
    set(CMAKE_BUILD_TYPE Release)
    # Нельзя оставлять переменную CMAKE_BUILD_TYPE пустой (подробности в Dviglo2D)
endif()

# Указываем Студии на то, что исходники в кодировке UTF-8.
# Это позволяет писать U'🍌'. У других компиляторов, похоже, нет с этим проблем
# https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2295r6.pdf
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Включаем поддержку папок в IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ============================================== Библиотека ==============================================

# Название таргета
set(target_name dv_std_lib)

# Создаём список файлов
file(GLOB source_files lib/*)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files})

# Делаем заголовочные файлы доступными приложениям
target_include_directories(${target_name} INTERFACE lib)

# Выводим больше предупреждений
if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4)
else() # GCC, Clang или MinGW
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)

    if(NOT MINGW AND DV_STD_LIB_UB_SANITIZER)
        target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
        target_link_options(${target_name} INTERFACE -fsanitize=undefined)
    endif()
endif()

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/lib FILES ${source_files})

# ============================================== Тестер ==============================================

if(DV_STD_LIB_TESTER)
    if(NOT root_file)
        # В IDE таргет будет отображаться в папке "тесты"
        set(CMAKE_FOLDER тесты)
    endif()

    # Название таргета
    set(target_name dv_std_lib_tester)

    # Создаём список файлов
    file(GLOB source_files tester/*)

    # Создаём приложение
    add_executable(${target_name} ${source_files})

    # Подключаем библиотеку
    target_link_libraries(${target_name} PRIVATE dv_std_lib)

    # Выводим больше предупреждений
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else() # GCC, Clang или MinGW
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)

        if(NOT MINGW AND DV_STD_LIB_UB_SANITIZER)
            target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
        endif()
    endif()

    # Статически линкуем библиотеки, чтобы не копировать dll-ки
    if(MINGW)
        target_link_options(${target_name} PRIVATE -static)
    endif()

    # Заставляем VS отображать дерево каталогов
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tester FILES ${source_files})

    # Включаем тестирование
    enable_testing()

    # Добавляем приложение в список тестируемых
    add_test(NAME ${target_name} COMMAND ${target_name})

    if(root_file)
        # В Visual Studio таргет будет назначен стартовым вместо ALL_BUILD,
        # чтобы потом не делать это вручную при отладке приложения
        set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target_name})
    endif()
endif()
