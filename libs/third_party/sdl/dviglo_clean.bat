:: Этот батник удаляет лишние файлы и папки после скачивания SDL

:: Меняем кодировку консоли на UTF-8
chcp 65001

:: В корне удаляем ненужные папки
rd /s /q .git
rd /s /q .github
rd /s /q android-project
rd /s /q build-scripts
:: cmake
rd /s /q docs
:: include
rd /s /q mingw
:: src
rd /s /q test
rd /s /q VisualC
rd /s /q VisualC-GDK
rd /s /q VisualC-WinRT
:: wayland-protocols
rd /s /q Xcode

:: В корне удаляем ненужные файлы
del .clang-format
del .clang-tidy
del .editorconfig
del .gitignore
del .wikiheaders-options
:: CREDITS.md
del INSTALL.md
del README.md
del Android.mk
del BUGS.txt
:: CMakeLists.txt
:: LICENSE.txt
:: README-SDL.txt
del WhatsNew.txt

:: Удаление всяких скриптов
del /s *.pl
del /s *.py
del /s *.sh

:: Удаляем все тесты
rd /s /q cmake\test
del include\SDL3\SDL_test*.h
rd /s /q src\test

:: Удаляем все конфигурационные h-файлы, так как
:: файл SDL_build_config.h.cmake генерируется с помощью SDL_build_config.h.cmake
del include\build_config\*.h

:: Удаляем файлы Visual Studio
del /s *.sln
del /s *.vcproj

:: Ждём нажатие Enter перед закрытием консоли
pause
