# Copyright (c) 2022-2023 the Dviglo project
# License: MIT

# Название таргета
set(TARGET_NAME dviglo)

# Отладочная версия библиотеки будут иметь суффикс _d.
# Не работает для экзешников
set(CMAKE_DEBUG_POSTFIX _d)

# Создаём список файлов
file(GLOB_RECURSE source_files *.cpp *.h)

if(DV_SHARED)
    # Создаём динамическую библиотеку
    add_library(${TARGET_NAME} SHARED ${source_files} ${generated_src_files})
    target_compile_definitions(${TARGET_NAME} PUBLIC DV_SHARED=1)
else()
    # Создаём статическую библиотеку
    add_library(${TARGET_NAME} STATIC ${source_files} ${generated_src_files})
endif()

# При компиляции библиотеки и при использовании библиотеки h-файлы отличаются
target_compile_definitions(${TARGET_NAME} PRIVATE DV_IS_BUILDING=1)

# Подключаем библиотеки из папки third_party
target_link_libraries(${TARGET_NAME} PUBLIC SDL3-static)

# Опции, которые не требуют ничего, кроме создания дефайнов
foreach(opt
            DV_TESTING # enable_testing() вызывается в common.cmake
            DV_WIN32_CONSOLE)
    if(${opt})
        target_compile_definitions(${TARGET_NAME} PUBLIC ${opt}=1)
    endif()
endforeach()

if(DV_ALL_WARNINGS)
    # Выводим больше предупреждений
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

# Делаем заголовочные файлы доступными таргетам, которые используют текущую библиотеку
target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# VS определяет дефайн _DEBUG в отладочной версии. Определяем этот дефайн также в GCC и MinGW.
# _DEBUG не определён в конфигурации RelWithDebInfo
if(NOT MSVC)
    target_compile_definitions(${TARGET_NAME} PUBLIC $<$<CONFIG:Debug>:_DEBUG>)
endif()

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
source_group("_generated" FILES ${generated_src_files})
