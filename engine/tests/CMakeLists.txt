# В IDE таргеты будут отображаться в папке "тесты"
set(CMAKE_FOLDER тесты)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result)

add_subdirectory(benchmark)
add_subdirectory(hello)
add_subdirectory(tester)
