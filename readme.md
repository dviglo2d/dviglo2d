[![DvStdLib](https://github.com/dviglo2d/dviglo2d/actions/workflows/dv_std_lib.yml/badge.svg)](https://github.com/dviglo2d/dviglo2d/actions/workflows/dv_std_lib.yml)
[![Engine](https://github.com/dviglo2d/dviglo2d/actions/workflows/engine.yml/badge.svg)](https://github.com/dviglo2d/dviglo2d/actions/workflows/engine.yml)
[![MinimalApp](https://github.com/dviglo2d/dviglo2d/actions/workflows/minimal_app.yml/badge.svg)](https://github.com/dviglo2d/dviglo2d/actions/workflows/minimal_app.yml)
[![DvBigInt](https://github.com/dviglo2d/dviglo2d/actions/workflows/dv_big_int.yml/badge.svg)](https://github.com/dviglo2d/dviglo2d/actions/workflows/dv_big_int.yml)
[![Main](https://github.com/dviglo2d/dviglo2d/actions/workflows/main.yml/badge.svg)](https://github.com/dviglo2d/dviglo2d/actions/workflows/main.yml)

# Dviglo2D

Игровой движок на основе SpriteBatch.

Поддерживаемые ОС: Linux и Windows 10+.

Скачивание репозитория с подмодулями в папку repo: `git clone --recurse-submodules --shallow-submodules https://github.com/dviglo2d/dviglo2d repo`.

Для компиляции в Linux запустите `build_linux.sh`. Необходимые зависимости: [engine/docs/building.md](engine/docs/building.md).

Для компиляции в Windows запустите `build_mingw.bat` или `build_vs.bat`. Вероятно в скриптах потребуется изменить пути к утилитам.
Подробнее: [engine/docs/building.md](engine/docs/building.md).

Лиценизя: [MIT](licenses/dviglo/license.txt).

## Структура репозитория

* Движок
  * Исходники: [engine/dviglo](engine/dviglo)
  * Собственные библиотеки, используемые в движке: [engine/libs](engine/libs)
  * Сторонние библиотеки, используемые в движке: [engine/third_party](engine/third_party)
  * Документация для движка: [engine/docs](engine/docs)
* Остальные сторонние библиотеки: [third_party](third_party)
* Шаблон приложения / Минимальное приложение: [minimal_app](minimal_app)
* Остальные собственные библиотеки
  * Длинная арифметика: [libs/dv_big_int](libs/dv_big_int)
  * Всячина для ImGui: [libs/dv_imgui](libs/dv_imgui)
  * Остальная всячина: [libs/dv_misc](libs/dv_misc)
* Туториалы
  * Поиск пути: [tutorials/pathfinding](tutorials/pathfinding)
  * Физика: [tutorials/physics](tutorials/physics)
* Игры
  * Пятнашки: [games/15_puzzle](games/15_puzzle)
  * Кликер: [games/clicker](games/clicker)
  * Клава: [games/klava](games/klava)
  * Леталка: [games/letalka](games/letalka)
  * Выжившие: [games/survivors](games/survivors)
* Утилиты
  * Генератор спрайтовых шрифтов: [utils/font_generator](utils/font_generator)
  * Спрайтовый манипулятор на Python: [utils/sprite_manipulator_py](utils/sprite_manipulator_py)
* Остальное
  * Общая документация: [docs](docs)
