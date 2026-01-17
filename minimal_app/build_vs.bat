:: Меняем кодировку консоли на UTF-8
chcp 65001 >nul

:: Указываем путь к cmake.exe. Без system32 в PATH ссылки на папки не создаются
set "PATH=%SystemRoot%\system32;c:\programs\cmake\bin"

set build_type=Debug
::set build_type=Release
::set build_type=MinSizeRel
::set build_type=RelWithDebInfo

set "generator=Visual Studio 17"

set "this_dir=%~dp0"
:: Удаляем обратный слэш в конце
set "this_dir=%this_dir:~0,-1%"

set "build_dir=%this_dir%/../../build_minimal_app_vs"

:: Генерируем проекты
cmake "%this_dir%" -B "%build_dir%" -G "%generator%" -A x64^
  -D DV_WIN32_CONSOLE=1 -D DV_OPENMP=1 -D DV_ENGINE_TESTS=0

:: Компилируем проекты
cmake --build "%build_dir%" --config %build_type% --parallel

:: Ждём нажатие Enter перед закрытием консоли
pause
