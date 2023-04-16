# В IDE таргеты будут отображаться в папке "утилиты"
set(CMAKE_FOLDER утилиты)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result)

dv_add_all_subdirs()
