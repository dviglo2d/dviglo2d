# Copyright (c) the Dviglo project
# License: MIT

# Указываем минимальную версию CMake
include(cmake/cmake_min_version.cmake)
cmake_minimum_required(VERSION ${dv_cmake_min_version})

# Название проекта
project(dviglo)

# Определяем, подключён ли движок как поддиректория
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(dv_is_subproject OFF)
else()
    set(dv_is_subproject ON)
endif()

include(cmake/common.cmake)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result)

add_subdirectory(libs)
add_subdirectory(samples)
add_subdirectory(tools)
