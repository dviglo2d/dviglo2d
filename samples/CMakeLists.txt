# Copyright (c) the Dviglo project
# License: MIT

if(DV_IS_SUBPROJECT)
    return()
endif()

if(NOT DV_SAMPLES)
    return()
endif()

# Помещаем примеры в отдельный проект
project("dv samples")

# В IDE таргеты будут отображаться в папке примеры
set(CMAKE_FOLDER примеры)

# Создаём папку result
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/result)

# Список папок с ресурсами для копирования в result
set(dir_names engine_data samples_data)

# Копируем папки с ресурсами в result, если нужно
foreach(dir_name ${dir_names})
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/result/${dir_name})
        dv_create_dir_link(${CMAKE_SOURCE_DIR}/result/${dir_name} ${CMAKE_BINARY_DIR}/result/${dir_name})
    endif()
endforeach()

# Добавляем все папки
dv_add_all_subdirs()
