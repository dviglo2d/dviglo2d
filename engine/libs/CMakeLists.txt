# В IDE таргеты будут отображаться в папке "библиотеки"
set(CMAKE_FOLDER библиотеки)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result)

# Добавляем подпапки
dv_add_all_subdirs()
