# В IDE таргеты будут отображаться в папке "игры"
set(CMAKE_FOLDER игры)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result)

dv_add_all_subdirs()
