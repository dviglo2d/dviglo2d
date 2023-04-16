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

# Добавляем все папки
dv_add_all_subdirs()
