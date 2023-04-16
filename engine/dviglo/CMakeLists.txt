# В IDE движок будет отображаться в папке "библиотеки"
set(CMAKE_FOLDER библиотеки)

# Название таргета
set(target_name dviglo)

# Создаём список файлов
file(GLOB_RECURSE source_files *.cpp *.hpp)

# Создаём статическую библиотеку
add_library(${target_name} STATIC ${source_files} ${generated_src_files})

# При компиляции библиотеки и при использовании библиотеки h-файлы отличаются
target_compile_definitions(${target_name} PRIVATE DV_IS_BUILDING=1)

# Отключаем лишние предупрежедения
if(MSVC)
    target_compile_definitions(${target_name} PRIVATE _CRT_SECURE_NO_WARNINGS=1)
endif()

# Подключаем библиотеки из папки third_party.
# Так как движок компилируется только в виде статической библиотеки,
# то приложения тоже могут использовать сторонние библиотеки без проблем
target_link_libraries(${target_name} PUBLIC SDL3-static SDL3_mixer-static glad stb glm pugixml freetype miniz)

# Опции, которые не требуют ничего, кроме создания дефайнов
foreach(opt
            DV_CTEST # enable_testing() вызывается в common.cmake
            DV_WIN32_CONSOLE)
    if(${opt})
        target_compile_definitions(${target_name} PUBLIC ${opt}=1)
    endif()
endforeach()

# Выводим больше предупреждений
if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4)
else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Делаем hpp-файлы движка доступными приложениям
target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# VS определяет дефайн _DEBUG в отладочной версии. Определяем этот дефайн также в GCC и MinGW.
# _DEBUG не определён в конфигурации RelWithDebInfo
if(NOT MSVC)
    target_compile_definitions(${target_name} PUBLIC $<$<CONFIG:Debug>:_DEBUG>)
endif()

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
source_group("_generated" FILES ${generated_src_files})
