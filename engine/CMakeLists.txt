# Указываем минимальную версию CMake
include(cmake/cmake_min_version.cmake)
cmake_minimum_required(VERSION ${dv_cmake_min_version})

# Название проекта
project(dviglo)

option(DV_ENGINE_TESTS "Тесты для движка" FALSE)

include(cmake/common.cmake)

add_subdirectory(third_party)
add_subdirectory(libs)
add_subdirectory(dviglo)

if(DV_ENGINE_TESTS)
    add_subdirectory(tests)
endif()
