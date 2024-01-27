:: Данный батник качает SDL нужной версии в папку repo

:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к git.exe
set "PATH=c:\program files\git\bin"

:: Качаем SDL 3 в папку repo
git clone https://github.com/libsdl-org/SDL repo

:: Возвращаем состояние репозитория к определённой версии
git -C repo reset --hard e71e16950a5591913bd15bc8f4d04c01bf654b72

:: Ждём нажатие Enter перед закрытием консоли
pause
