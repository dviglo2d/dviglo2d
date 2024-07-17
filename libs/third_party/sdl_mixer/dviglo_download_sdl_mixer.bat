:: Данный батник качает SDL_mixer нужной версии в папку repo

:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к git.exe
set "PATH=c:\program files\git\bin"

:: Качаем SDL_mixer 3 в папку repo
git clone https://github.com/libsdl-org/SDL_mixer repo

:: Возвращаем состояние репозитория к определённой версии
git -C repo reset --hard 96ea69fce472dcef7c638199aef8e3c81200573a

:: Ждём нажатие Enter перед закрытием консоли
pause
