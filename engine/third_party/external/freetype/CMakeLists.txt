# Перезаписываем опции
# https://stackoverflow.com/questions/39588836/why-do-i-need-force-to-override-a-cmake-option
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)

# Подавляем "include(CPack)"
# https://cmake.org/pipermail/cmake/2012-April/050023.html
macro(include file)
   if(NOT ${file} STREQUAL CPack)
      _include(${file} ${ARGN})
   endif()
endmacro()

# Подавляем "cmake_minimum_required(VERSION 3.0...3.5)"
macro(cmake_minimum_required)
endmacro()

add_subdirectory(repo)
