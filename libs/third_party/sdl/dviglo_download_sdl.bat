:: Данный батник качает SDL нужной версии в папку repo

:: Меняем кодировку консоли на UTF-8
chcp 65001

:: Путь к git.exe
set "PATH=c:\program files\git\bin"

:: Качаем SDL 3 в папку repo
git clone https://github.com/libsdl-org/SDL repo

:: Возвращаем состояние репозитория к определённой версии
git -C repo reset --hard 06bea1eb5544e778c1258d3d7f47f6d06110c379

:: Ждём нажатие Enter перед закрытием консоли
pause
