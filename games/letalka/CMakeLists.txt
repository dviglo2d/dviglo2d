# Название таргета
set(target_name letalka)

# Создаём список файлов
file(GLOB_RECURSE source_files src/*.cpp src/*.hpp)

# Создаём приложение
add_executable(${target_name} ${source_files})

if(NOT DV_WIN32_CONSOLE)
    # Используем точку входа WinMain()
    set_property(TARGET ${target_name} PROPERTY WIN32_EXECUTABLE TRUE)
endif()

# Выводим больше предупреждений
if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4)
else()
    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Подключаем библиотеки
target_link_libraries(${target_name} PRIVATE dviglo entt)

# Копируем динамические библиотеки в папку с приложением
dv_copy_shared_libs_to_bin_dir(${target_name})

# Компилируем шейдеры
add_dependencies(${target_name} dv_compile_engine_shaders)

# Копируем ресурсы в result
add_custom_command(TARGET ${target_name} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_newer
                       ${CMAKE_BINARY_DIR}/shaders
                       $<TARGET_FILE_DIR:${target_name}>/engine_data/shaders)

add_custom_command(TARGET ${target_name} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_newer
                       ${engine_dir}/engine_test_data
                       $<TARGET_FILE_DIR:${target_name}>/engine_test_data)

add_custom_command(TARGET ${target_name} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_newer
                       ${CMAKE_CURRENT_SOURCE_DIR}/letalka_data
                       $<TARGET_FILE_DIR:${target_name}>/letalka_data)

# Добавляем приложение в список тестируемых
add_test(NAME ${target_name} COMMAND ${target_name} -duration 5)

# Заставляем VS отображать дерево каталогов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${source_files})
