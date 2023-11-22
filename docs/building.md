# Сборка

Поддерживаемые OS:
1. Windows
2. Linux

Необходимый софт:
1. Git для скачивания исходников из репозитория
2. один из поддерживаемых компиляторов (об этом ниже)
3. CMake для генерации проектов для используемого компилятора

В Windows `Git` можно скачать [отсюда](https://git-scm.com), а `CMake` [отсюда](https://cmake.org).

--------------------------------------------------

# Сборка в Linux

Поддерживаемые компиляторы:
1. GCC 13 (GCC 11 не поддерживает `shared_ptr<T[]>`, GCC 12 не поддерживает `std::format`)
2. Clang 13 (проверена версия 13.0.1). Clang использует заголовки от GCC (libstdc++),
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

# Без libxrandr-dev не получится узнать список поддерживаемых разрешений
sudo apt install libgl1-mesa-dev libxrandr-dev libasound2-dev
```

## Скачивание и компиляция движка

```
#!/bin/sh

# Качаем репозиторий в папку repo
git clone https://github.com/dviglo2d/dviglo2d repo

# Создаём проекты для GCC 13 в папке build, используя конфиг CMakeLists.txt из папки repo
cmake repo -B build -G "Unix Makefiles" \
    -D CMAKE_C_COMPILER=gcc-13 -D CMAKE_CXX_COMPILER=g++-13 \
    -D CMAKE_BUILD_TYPE=Release

# CMAKE_BUILD_TYPE может быть Debug, Release, MinSizeRel или RelWithDebInfo

# Если хотите использовать Clang, а не GCC, тогда -D CMAKE_C_COMPILER=clang-13 -D CMAKE_CXX_COMPILER=clang++-13

# Компилируем проекты в папке build
cmake --build build
```

Но удобнее компилировать с помощью [Visual Studio Code](vscode.md).

--------------------------------------------------

# Сборка в Windows

Поддерживаемые компиляторы:

1. Visual Studio 2022
2. MinGW-w64 13.2 из пакета MSYS2

## Visual Studio

```
:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Указываем пути к необходимым утилитам
set "PATH=c:\programs\cmake\bin;c:\program files\git\bin"

:: Качаем репозиторий в папку repo
git clone https://github.com/dviglo2d/dviglo2d repo

:: Создаём проекты для Visual Studio 2022 в папке build, используя конфиг CMakeLists.txt из папки repo
cmake repo -B build -G "Visual Studio 17" -A x64

:: Компилируем проекты в папке build
cmake --build build --config Release

:: --config может быть Debug, Release, MinSizeRel или RelWithDebInfo

:: Ждём нажатие Enter перед закрытием консоли
pause
```

Использовать компилятор от Microsoft можно и без установки `Visual Studio` IDE.
Достаточно установить [Build Tools](https://visualstudio.microsoft.com/downloads/?q=build+tools).
А в качестве IDE использовать [Visual Studio Code](vscode.md).

## MinGW-w64

### Установка

1. Качаем и устанавливаем MSYS2 с <https://www.msys2.org>
2. Запускаем `MSYS2 UCRT64` из меню `Пуск`
3. Вводим `pacman -S mingw-w64-ucrt-x86_64-toolchain`

Источник: https://stackoverflow.com/questions/30069830/how-can-i-install-mingw-w64-and-msys2

### Скачивание и компиляция движка

```
:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Указываем пути к необходимым утилитам
set "PATH=c:\windows\system32;c:\program files\git\bin;c:\programs\cmake\bin;c:\msys64\ucrt64\bin"

:: Качаем репозиторий в папку repo
git clone https://github.com/dviglo2d/dviglo2d repo

:: Создаём проекты для MinGW-w64 в папке build, используя конфиг CMakeLists.txt из папки repo
cmake repo -B build -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=Release

:: CMAKE_BUILD_TYPE может быть Debug, Release, MinSizeRel или RelWithDebInfo

:: Компилируем проекты в папке build
cmake --build build

:: Ждём нажатие Enter перед закрытием консоли
pause
```
