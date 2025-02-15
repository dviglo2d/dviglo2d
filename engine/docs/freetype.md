Family - гарнитура - семейство шрифтов. Например "Courier".

Face - начертание - конкретный шрифт. Например "Courier Bold".

Glyph - глиф - изображение определенного символа из шрифта.

Кернинг - поправка расстояния между парой конкретных символов.

Если использовать функцию FT_Set_Char_Size(), то
размер шрифта задается в пунктах (points). Один пункт = 1/72 дюйма.
Стандартное разрешение Windows 96x96 dpi (пикселей на дюйм),
значит 12pt = 12 * 96 / 72 = 16 пикселей. Это размер воображаемой рамки
вокруг символа. Как символ расположен внутри рамки, зависит от шрифта.

Приложение использует функцию FT_Set_Pixel_Sizes(), поэтому абзац выше не важен.

В библиотеке координаты и размеры хранятся в целых числах с единицей измерения
1/64 (fixed-point). Поэтому размеры нужно умножать на 64.
Операция сдвига ">> 6" равносильна делению на 64 (0b1000000 превращается в 1).
Формат называется 26.6 (26 бит на целую часть, 6 на дробную).
Еще есть формат 16.16 (16 бит на целую часть, 16 на дробную). Чтобы округлить
такое число до нормального целого, его нужно разделить на 65536.

У каждого FT_Face есть один FT_GlyphSlot (face->glyph), который хранит информацию о последнем загруженном глифе.

См. также: https://www.freetype.org/freetype2/docs/documentation.html
