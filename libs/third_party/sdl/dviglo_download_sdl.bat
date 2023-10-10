:: Данный батник качает SDL нужной версии в папку repo

:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к git.exe
set "PATH=c:\program files\git\bin"

:: Качаем SDL 3 в папку repo
git clone https://github.com/libsdl-org/SDL repo

:: Возвращаем состояние репозитория к определённой версии
git -C repo reset --hard 55a1458ed00644f5fccbeb6b95d4e567240e6284

:: Ждём нажатие Enter перед закрытием консоли
pause
