# Copyright (c) 2022-present the Dviglo project
# License: MIT

if(DV_IS_SUBPROJECT)
    return()
endif()

# Помещаем инструменты в отдельный проект
project("dv tools")

# В IDE таргеты будут отображаться в папке инструменты
set(CMAKE_FOLDER инструменты)

# Папка для экзешников и динамических библиотек
dv_set_bin_dir(${dviglo_BINARY_DIR}/bin/tools)

add_subdirectory(tests)
