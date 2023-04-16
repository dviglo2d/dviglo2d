# Путь к папке движка
set(engine_dir "${CMAKE_CURRENT_SOURCE_DIR}/../engine")

# Указываем минимальную версию CMake
include(${engine_dir}/cmake/cmake_min_version.cmake)
cmake_minimum_required(VERSION ${dv_cmake_min_version})

# Название проекта
project(app)

include(${engine_dir}/cmake/common.cmake)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result)

# Подключаем движок
add_subdirectory(${engine_dir} engine)

# Название таргета
set(target_name app)

# Создаём список файлов
file(GLOB_RECURSE source_files src/*.cpp src/*.hpp)

# Создаём приложение
add_executable(${target_name} ${source_files})

if(NOT DV_WIN32_CONSOLE)
    # Используем точку входа WinMain()
    set_property(TARGET ${target_name} PROPERTY WIN32_EXECUTABLE TRUE)
endif()

# Выводим больше предупреждений
if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4)
else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Подключаем библиотеку
target_link_libraries(${target_name} PRIVATE dviglo)

# Копируем динамические библиотеки в папку с приложением
dv_copy_shared_libs_to_bin_dir(${target_name})

# Копируем папки с ресурсами движка в result, если нужно
set(dir_names engine_data engine_test_data)
foreach(dir_name ${dir_names})
    dv_create_dir_link(${engine_dir}/result/${dir_name} ${dv_bin_dir}/${dir_name})
endforeach()

# Копируем папки с ресурсами приложения в result, если нужно
set(dir_names app_data)
foreach(dir_name ${dir_names})
    dv_create_dir_link(${CMAKE_CURRENT_SOURCE_DIR}/${dir_name} ${dv_bin_dir}/${dir_name})
endforeach()

# Добавляем приложение в список тестируемых
add_test(NAME ${target_name} COMMAND ${target_name} -duration 5)

# Заставляем Visual Studio отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${source_files})

# В Visual Studio таргет будет назначен стартовым вместо ALL_BUILD,
# чтобы потом не делать это вручную при отладке приложения
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target_name})
