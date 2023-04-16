:: Меняем кодировку консоли на UTF-8
chcp 65001 >nul

:: Указываем путь к cmake.exe и MinGW. Без system32 в PATH ссылки на папки не создаются
set "PATH=%SystemRoot%\system32;c:\programs\cmake\bin;c:\msys64\ucrt64\bin"

set build_type=Debug
::set build_type=Release
::set build_type=MinSizeRel
::set build_type=RelWithDebInfo

set "this_dir=%~dp0"
:: Удаляем обратный слэш в конце
set "this_dir=%this_dir:~0,-1%"

set "build_dir=%this_dir%/../../build_engine_mingw"

:: Генерируем проекты
cmake "%this_dir%" -B "%build_dir%" -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=%build_type%^
  -D DV_WIN32_CONSOLE=1 -D DV_OPENMP=1 -D DV_ENGINE_TESTS=1

:: Компилируем проекты
cmake --build "%build_dir%" --parallel

:: Ждём нажатие Enter перед закрытием консоли
pause
