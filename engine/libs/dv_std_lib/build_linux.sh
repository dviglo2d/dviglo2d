#!/bin/sh

build_type="-D CMAKE_BUILD_TYPE=Debug"
#build_type="-D CMAKE_BUILD_TYPE=Release"
#build_type="-D CMAKE_BUILD_TYPE=MinSizeRel"
#build_type="-D CMAKE_BUILD_TYPE=RelWithDebInfo"

compiler="-D CMAKE_C_COMPILER=gcc-13 -D CMAKE_CXX_COMPILER=g++-13"
#compiler="-D CMAKE_C_COMPILER=clang-18 -D CMAKE_CXX_COMPILER=clang++-18"

this_dir=$(dirname "$0")

build_dir="$this_dir/../../../../build_std_lib_int"

# Генерируем проекты
cmake "$this_dir" -B "$build_dir" -G "Unix Makefiles" $build_type $compiler \
  -D DV_STD_LIB_TESTER=1 -D DV_STD_LIB_UB_SANITIZER=1

# Компилируем проекты
cmake --build "$build_dir" --parallel
