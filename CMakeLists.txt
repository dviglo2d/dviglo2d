# Путь к папке движка
set(engine_dir "${CMAKE_SOURCE_DIR}/engine")

# Указываем минимальную версию CMake
include(${engine_dir}/cmake/cmake_min_version.cmake)
cmake_minimum_required(VERSION ${dv_cmake_min_version})

# Название проекта
project(all)

include(${engine_dir}/cmake/common.cmake)

add_subdirectory(engine)
add_subdirectory(third_party)
add_subdirectory(libs)
add_subdirectory(games)
add_subdirectory(utils)
add_subdirectory(tutorials)
