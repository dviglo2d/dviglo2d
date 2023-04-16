* [Composition over inheritance](https://en.wikipedia.org/wiki/Composition_over_inheritance)
* [UTF-8 Everywhere](https://utf8everywhere.org)
* Максимум комментариев в коде.
  Хороший пример - [Serious Engine](https://github.com/Croteam-official/Serious-Engine)
* Стараться избегать использования `auto`
* Структуры с размером 16 байт и меньше передавать по значению, а не по ссылке
* Не использовать исключения
* Предпочитать знаковые типы для индексов и размеров:
  * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1428r0.pdf
  * https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1491r0.pdf
  * https://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf
  * Bullet использует int: https://github.com/bulletphysics/bullet3/blob/master/src/LinearMath/btAlignedObjectArray.h
  * C# использует int: https://docs.microsoft.com/en-us/dotnet/api/system.array.length?view=net-6.0
  * Qt использует int: https://doc.qt.io/qt-5/qlist.html
    (был беззнаковый в [Qt3](https://doc.qt.io/archives/3.3/qvaluevector.html#size_type),
    стал знаковый в [Qt4](https://doc.qt.io/archives/4.3/qvector.html#size_type-typedef))
* Внутри движка используется предумноженная альфа (premultiplied alpha) вместо обычной (straight alpha).
  Это помогает избавиться от ореолов вокруг спрайтов при фильтрации
* Внутри движка используется линейное цветовое пространство и только при выводе в окно преобразуется в sRGB.
  Спрайты лучше хранить в линейном RGB, чтобы избежать потерь при преобразовании sRGB -> RGB
