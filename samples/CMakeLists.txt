# Copyright (c) 2022-2023 the Dviglo project
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

file(MAKE_DIRECTORY ${dviglo_BINARY_DIR}/bin)

# Список папок для копирования в bin
set(dir_names data)

# Копируем папки в bin, если нужно
foreach(dir_name ${dir_names})
    if(NOT EXISTS ${dviglo_BINARY_DIR}/bin/${dir_name})
        dv_create_dir_link(${dviglo_SOURCE_DIR}/bin/${dir_name} ${dviglo_BINARY_DIR}/bin/${dir_name})
    endif()
endforeach()

# Добавляем все папки
dv_add_all_subdirs()
