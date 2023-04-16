# Сборка

Поддерживаемые OS:
1. Windows 10 версии 1803 (10.0.17134.0) или выше
2. Linux

Необходимый софт:
1. Git для скачивания исходников из репозитория
2. Один из поддерживаемых компиляторов (об этом ниже)
3. CMake для генерации проектов для используемого компилятора

В Windows `Git` можно скачать [отсюда](https://git-scm.com), а `CMake` [отсюда](https://cmake.org).

--------------------------------------------------

# Сборка в Linux

Поддерживаемые компиляторы:
1. GCC 13+
2. Clang 18+. Clang использует библиотеки от GCC (libstdc++),
   поэтому GCC 13 всё равно должен быть установлен

## Установка GCC 13 в Linux Mint 21.2

```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update -y
sudo apt install gcc-13 g++-13 -y
```

Источники:
* https://github.com/actions/runner-images/blob/50625a842ad09f741de40651332bdc6064c7f0a9/images/ubuntu/scripts/build/gcc.sh
* https://phoenixnap.com/kb/install-gcc-ubuntu

Если что, подключенный PPA-репозиторий можно удалить с помощью программы `Источники приложений`
или командой `sudo add-apt-repository --remove ppa:ubuntu-toolchain-r/test`.
Не забудьте после этого сделать `sudo apt-get update` и `sudo apt-get upgrade`.

## Установка необходимых зависимостей

```
sudo apt update

# Без libxrandr-dev не получится узнать список поддерживаемых разрешений.
# Без libxi-dev игра не сможет захватывать курсор
sudo apt install libgl1-mesa-dev libxrandr-dev libxi-dev libasound2-dev libxcursor-dev libxss-dev libxtst-dev
```

Полный список зависимостей SDL: <https://github.com/dviglo2d/sdl/blob/main/docs/README-linux.md>

Для Clang дополнительно нужно установить OpenMP:

```
# Установка OpenMP для Clang 18
sudo apt install libomp-18-dev
```

Это добавит в зависимости пакет libomp5-18t64, который должен быть установлен
у конечного пользователя. Впрочем, движок можно скомпилировать и без поддержки OpenMP: `DV_OPENMP=0`.

## Сборка

Запустите скрипт `build_linux.sh`.

--------------------------------------------------

# Сборка в Windows

Поддерживаемые компиляторы:

1. Visual Studio 2022
2. MinGW-w64 13.2 из пакета MSYS2

## Visual Studio

Для сборки запустите скрипт `build_vs.bat`, предварительно исправив в нём пути.

Использовать компилятор от Microsoft можно и без установки `Visual Studio` IDE.
Достаточно установить [Build Tools](https://visualstudio.microsoft.com/downloads/?q=build+tools).
А в качестве IDE использовать [Visual Studio Code](../../docs/vscode.md).

## MinGW-w64

### Установка компилятора

1. Качаем и устанавливаем MSYS2 с <https://www.msys2.org>
2. Запускаем `MSYS2 UCRT64` из меню `Пуск`
3. Вводим `pacman -S mingw-w64-ucrt-x86_64-toolchain`

Источник: https://stackoverflow.com/questions/30069830/how-can-i-install-mingw-w64-and-msys2

### Сборка

Запустите скрипт `build_mingw.bat`, предварительно исправив в нём пути.
