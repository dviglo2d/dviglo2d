# Перезаписываем опции
# https://stackoverflow.com/questions/39588836/why-do-i-need-force-to-override-a-cmake-option
set(SDL_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE) # Нужна только статическая версия библиотеки
set(SDL_STATIC ON CACHE BOOL "" FORCE) # Нужна только статическая версия библиотеки
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE) # Тесты не нужны

# Отключаем встроенный 2D-рендерер
set(SDL_RENDER_DEFAULT OFF)

# Запрещаем SDL использовать git
# https://github.com/libsdl-org/SDL/issues/7561
set(SDL_REVISION "unknown" CACHE STRING "" FORCE)

add_subdirectory(repo)
