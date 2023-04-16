add_subdirectory(repo)

# Подавляем предупреждения
if(NOT MSVC) # GCC, Clang или MinGW
    target_compile_options(miniz PRIVATE -Wno-unused-function)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(miniz PRIVATE -Wno-newline-eof)
endif()
