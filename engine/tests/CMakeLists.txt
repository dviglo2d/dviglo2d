# В IDE таргеты будут отображаться в папке "тесты"
set(CMAKE_FOLDER тесты)

# Папка для экзешников
set(result_dir "${CMAKE_BINARY_DIR}/result")

# Создаём папку result
file(MAKE_DIRECTORY ${result_dir})

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${result_dir})

# Список папок с ресурсами для копирования в result
set(dir_names engine_data engine_test_data)

# Копируем папки с ресурсами в result, если нужно
foreach(dir_name ${dir_names})
    dv_create_dir_link(${CMAKE_CURRENT_SOURCE_DIR}/../result/${dir_name} ${CMAKE_BINARY_DIR}/result/${dir_name})
endforeach()

add_subdirectory(hello)
add_subdirectory(tester)
