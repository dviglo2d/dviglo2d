# Copyright (c) the Dviglo project
# License: MIT

# Указываем минимальную версию CMake
include(cmake/cmake_min_version.cmake)
cmake_minimum_required(VERSION ${DV_CMAKE_MIN_VERSION})

# Название проекта
project(dviglo)

# Определяем, подключён ли движок как поддиректория
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(DV_IS_SUBPROJECT OFF)
else()
    set(DV_IS_SUBPROJECT ON)
endif()

include(cmake/common.cmake)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result)

add_subdirectory(libs)
add_subdirectory(samples)
add_subdirectory(tools)
