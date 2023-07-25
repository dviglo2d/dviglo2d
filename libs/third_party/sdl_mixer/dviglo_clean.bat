:: Этот батник удаляет лишние файлы и папки после скачивания SDL

:: Меняем кодировку консоли на UTF-8
chcp 65001

:: В корне удаляем ненужные папки
rd /s /q .git
rd /s /q .github
rd /s /q build-scripts
:: cmake
rd /s /q examples
rd /s /q external
:: include
rd /s /q mingw
:: src
rd /s /q VisualC
rd /s /q VisualC-WinRT
rd /s /q Xcode

:: В корне удаляем ненужные файлы
del .clang-format
del .editorconfig
del .gitignore
del .gitmodules
del .wikiheaders-options
del README-versions.md
del release_checklist.md
del Android.mk
del CHANGES.txt
:: CMakeLists.txt
:: LICENSE.txt
del README.txt

:: Удаляем тест
rd /s /q cmake\test

:: Ждём нажатие Enter перед закрытием консоли
pause
