# Поддерживаемые компиляторы

Linux:
1. GCC 13 (GCC 11 не поддерживает `shared_ptr<T[]>`, GCC 12 не поддерживает `std::format`)
2. Clang 13 (проверена версия 13.0.1). Clang использует заголовки от GCC (libstdc++),
   поэтому GCC 13 всё равно должен быть установлен

Windows:
1. Visual Studio 2022
2. mingw-w64 12.1 из пакета MSYS2
