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

# =================== Компиляция шейдеров в SPIR-V ===================

file(GLOB shader_src_files CONFIGURE_DEPENDS shaders_src/*.vert shaders_src/*.frag shaders_src/*.comp)
set(shader_spv_files "")

foreach(shader_src_file ${shader_src_files})
    get_filename_component(file_name ${shader_src_file} NAME)
    set(shader_spv_file ${CMAKE_BINARY_DIR}/shaders/${file_name}.spv)

    add_custom_command(OUTPUT ${shader_spv_file}
                       COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
                       COMMAND ${dv_glslc_executable} -O --target-spv=spv1.0 ${shader_src_file} -o ${shader_spv_file}
                       DEPENDS ${shader_src_file}
                       VERBATIM)
    list(APPEND shader_spv_files ${shader_spv_file})
endforeach()

add_custom_target(dv_compile_engine_shaders DEPENDS ${shader_spv_files})
