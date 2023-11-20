# Поддерживаемые компиляторы

Linux:
1. GCC 13 (GCC 11 не поддерживает `shared_ptr<T[]>`, GCC 12 не поддерживает `std::format`)
2. Clang 13 (проверена версия 13.0.1). Clang использует заголовки от GCC (libstdc++),
   поэтому GCC 13 всё равно должен быть установлен

Windows:
1. Visual Studio 2022
2. mingw-w64 12.1 из пакета MSYS2

# Установка GCC 13 в Linux Mint 21.2

```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update -y
sudo apt install gcc-13 g++-13 -y
```

Источники:
* https://github.com/actions/runner-images/blob/50625a842ad09f741de40651332bdc6064c7f0a9/images/ubuntu/scripts/build/gcc.sh
* https://phoenixnap.com/kb/install-gcc-ubuntu

Если что, подключенный PPA-репозиторий можно удалить с помощью программы `Источники приложений`.
