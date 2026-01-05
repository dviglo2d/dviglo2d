# Аналог #pragma once
include_guard(GLOBAL)

# Если используется одноконфигурационный генератор
# и конфигурация не указана
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    # то конфигурацией по умолчанию будет Release
    set(CMAKE_BUILD_TYPE Release)

    # Нельзя оставлять переменную CMAKE_BUILD_TYPE пустой,
    # так как при этом не будут заданы флаги GCC и MinGW:
    # * Пустая строка: CXX_FLAGS = -std=c++23
    # * Release: CXX_FLAGS = -O3 -DNDEBUG -std=c++23
    # * Debug: CXX_FLAGS = -g -std=c++23
    # * RelWithDebInfo: CXX_FLAGS = -O2 -g -DNDEBUG -std=c++23
    # * MinSizeRel: CXX_FLAGS = -Os -DNDEBUG -std=c++23
    # Флаги можно посмотреть в файле build/engine/dviglo/CMakeFiles/dviglo.dir/flags.make
endif()

# Предупреждаем об in-source build
if(CMAKE_BINARY_DIR MATCHES "^${CMAKE_SOURCE_DIR}")
    message(WARNING "Генерировать проекты в папке с иходниками - плохая идея")
endif()

# Выводим параметры командной строки
get_cmake_property(CACHE_VARS CACHE_VARIABLES)
foreach(cache_var ${CACHE_VARS})
    get_property(CACHE_VAR_HELPSTRING CACHE ${cache_var} PROPERTY HELPSTRING)

    if(CACHE_VAR_HELPSTRING STREQUAL "No help, variable specified on the command line.")
        get_property(cache_var_type CACHE ${cache_var} PROPERTY TYPE)

        if(cache_var_type STREQUAL "UNINITIALIZED")
            set(cache_var_type)
        else()
            set(cache_var_type ":${cache_var_type}")
        endif()

        set(cmake_args "${cmake_args} -D ${cache_var}${cache_var_type}=\"${${cache_var}}\"")
    endif ()
endforeach ()
message(STATUS "Параметры командной строки:${cmake_args}")

# Версия стандарта C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Указываем Студии на то, что исходники в кодировке UTF-8.
# Это позволяет писать U'🍌'. У других компиляторов, похоже, нет с этим проблем.
# https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2295r6.pdf
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Включаем поддержку папок в IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Включаем опцию /MT для всех таргетов
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Включаем многопоточную сборку для Студии
if(MSVC)
    add_compile_options(/MP)
endif()

# ==================== Опции движка ====================

# Получаем доступ к макросу cmake_dependent_option
include(CMakeDependentOption)

option(DV_CTEST "Поддержка CTest" FALSE)
cmake_dependent_option(DV_WIN32_CONSOLE "Использовать main(), а не WinMain()" FALSE "WIN32" FALSE) # Не на Windows всегда FALSE
option(DV_OPENMP "Поддержка OpenMP" TRUE)

if(DV_CTEST)
    enable_testing() # Должно быть в корневом CMakeLists.txt
endif()

# ==================== Утилиты ====================

# Добавляет все поддиректории, в которых есть CMakeLists.txt.
# Использование: dv_add_all_subdirs() или dv_add_all_subdirs(EXCLUDE_FROM_ALL)
function(dv_add_all_subdirs)
    # Список файлов и подпапок
    file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

    foreach(child ${children})
        # Если не директория, то пропускаем
        if(NOT IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
            continue()
        endif()

        # Если в папке нет CMakeLists.txt, то пропускаем
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt)
            continue()
        endif()

        # Функция dv_add_all_subdirs() ожидает 0 аргументов.
        # Все лишние аргументы помещаются в ARGN
        add_subdirectory(${child} ${ARGN})
    endforeach()
endfunction()

# Создаёт ссылку для папки. Если ссылку создать не удалось, то копирует папку
function(dv_create_dir_link from to)
    if(EXISTS ${to})
        return()
    endif()

    if(NOT CMAKE_HOST_WIN32)
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${from} ${to}
                        OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE RESULT)
    else()
        # Не используем create_symlink в Windows, так как создание symbolic links
        # [требует админских прав](https://ss64.com/nt/mklink.html),
        # а поддержка junctions из CMake
        # [была удалена](https://gitlab.kitware.com/cmake/cmake/-/merge_requests/7530)
        string(REPLACE / \\ from ${from})
        string(REPLACE / \\ to ${to})
        execute_process(COMMAND cmd /C mklink /J ${to} ${from}
                        OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE RESULT)
    endif()

    if(NOT RESULT EQUAL 0)
        # Причиной неудачи может быть перезаписанная переменная PATH, в которой нет %SystemRoot%\system32
        message("Не удалось создать ссылку для папки, поэтому копируем папку")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory  ${from} ${to})
    endif()
endfunction()

# Куда будут помещены следующие скомпилированные экзешники и динамические библиотеки.
# Функцию нужно вызывать перед созданием таргетов
function(dv_set_bin_dir bin_dir)
    # Переменная будет доступна после вызова функции
    set(dv_bin_dir ${bin_dir} PARENT_SCOPE)

    # Создаём папку
    file(MAKE_DIRECTORY ${bin_dir})

    # Для одноконфигурационных генераторов (MinGW)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bin_dir} PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${bin_dir} PARENT_SCOPE)

    # Для многоконфигурационных генераторов (Visual Studio)
    foreach(config_name ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config_name} config_name)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_name} ${bin_dir} PARENT_SCOPE)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_name} ${bin_dir} PARENT_SCOPE)
    endforeach()
endfunction()

# После компиляции копирует dll-ки к экзешнику
function(dv_copy_shared_libs_to_bin_dir exe_target_name)
    set(dlls "")

    if(MINGW)
        # dll-ки MinGW находятся рядом с компилятором, поэтому определяем путь к компилятору
        execute_process(COMMAND where x86_64-w64-mingw32-gcc.exe
                        OUTPUT_VARIABLE mingw_fullpath)

        # Команда where может вернуть несколько строк, если MinGW установлен в разные места.
        # Преобразуем вывод команды в список и получаем нулевой элемент
        string(REPLACE "\n" ";" mingw_fullpath ${mingw_fullpath})
        list(GET mingw_fullpath 0 mingw_fullpath)

        cmake_path(GET mingw_fullpath PARENT_PATH mingw_dir)

        list(APPEND dlls ${mingw_dir}/libgcc_s_seh-1.dll
                         ${mingw_dir}/libstdc++-6.dll
                         ${mingw_dir}/libwinpthread-1.dll)

        if(DV_OPENMP)
            list(APPEND dlls ${mingw_dir}/libgomp-1.dll)
        endif()
    endif()

    if(NOT dlls STREQUAL "")
        add_custom_command(TARGET ${exe_target_name} POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dlls} $<TARGET_FILE_DIR:${exe_target_name}>
                           COMMAND_EXPAND_LISTS)
    endif()
endfunction()
