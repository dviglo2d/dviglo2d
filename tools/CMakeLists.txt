# Copyright (c) the Dviglo project
# License: MIT

if(dv_is_subproject)
    return()
endif()

# Помещаем инструменты в отдельный проект
project("dv tools")

# В IDE таргеты будут отображаться в папке инструменты
set(CMAKE_FOLDER инструменты)

# Указываем папку для следующих скомпилированных экзешников
dv_set_bin_dir(${CMAKE_BINARY_DIR}/result/tools)

add_subdirectory(tester)
