:: Данный батник качает SDL_mixer нужной версии в папку repo

:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к git.exe
set "PATH=c:\program files\git\bin"

:: Качаем SDL_mixer 3 в папку repo
git clone https://github.com/libsdl-org/SDL_mixer repo

:: Возвращаем состояние репозитория к определённой версии
git -C repo reset --hard e645f794936819b944d70f0e64e6554e4e07a3f2

:: Ждём нажатие Enter перед закрытием консоли
pause
