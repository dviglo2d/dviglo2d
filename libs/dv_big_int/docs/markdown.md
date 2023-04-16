# Полезные символы

Символы, которые используются в документации, но которых нет на клавиатуре:

| Символ | Unicode | В HTML                                              | Источники | Примечания |
|:------:|:-------:|-----------------------------------------------------|------------|-----------|
| −      | U+2212  | `&#8722;` `&#x2212;` `&minus;`                      | [−](https://www.compart.com/en/unicode/U+2212), [Минус](https://ru.wikipedia.org/wiki/Минус), [Плюс и минус](https://ru.wikipedia.org/wiki/Знаки_плюса_и_минуса)    | Не путать с [дефисом-минусом](https://ru.wikipedia.org/wiki/Дефис#Дефис_и_компьютеры) на клавиатуре. Минус используется в формулах, а дефис-минус в реальном коде |
| —      | U+2014  | `&#8212;` `&#x2014;` `&mdash;`                      | [—](https://www.compart.com/en/unicode/U+2014), [Тире](https://ru.wikipedia.org/wiki/Тире)    | Длинное тире обозначает пропуск в предложении    |
| ·      | U+00B7  | `&#183;` `&#xB7;` `&centerdot;` `&middot;`          | [·](https://www.compart.com/en/unicode/U+00B7), [Знак умножения](https://ru.wikipedia.org/wiki/Знак_умножения)    | [⋅](https://www.compart.com/en/unicode/U+22C5) не используется, так как есть не во всех шрифтах |
| …      | U+2026  | `&#8230;` `&#x2026;` `&mldr;` `&hellip;`            | […](https://www.compart.com/en/unicode/U+2026), [Многоточие](https://ru.wikipedia.org/wiki/Многоточие), [Ellipsis](https://en.wikipedia.org/wiki/Ellipsis)    |    |
| ⇒      | U+21D2  | `&#8658;` `&#x21D2;` `&Rightarrow;`                 | [⇒](https://www.compart.com/en/unicode/U+21D2)    |    |
| ⇔      | U+21D4  | `&#8660;` `&#x21D4;` `&DoubleLeftRightArrow;`       | [⇔](https://www.compart.com/en/unicode/U+21D4)    |    |
| ≤      | U+2264  | `&#8804;` `&#x2264;` `&leq;`                        | [≤](https://www.compart.com/en/unicode/U+2264)    | [⩽](https://www.compart.com/en/unicode/U+2A7D) не используется, так как есть не во всех шрифтах |
| ≥      | U+2265  | `&#8805;` `&#x2265;` `&GreaterEqual;`               | [≥](https://www.compart.com/en/unicode/U+2265)    | [⩾](https://www.compart.com/en/unicode/U+2A7E) не используется, так как есть не во всех шрифтах |

Дополнительный источник: https://ru.wikipedia.org/wiki/Таблица_математических_символов

# HTML-теги

* Разрыв строки в любом месте: `<br>`
* `10<sup>4</sup>` выглядит как 10<sup>4</sup>
* `10<sub>4</sub>` выглядит как 10<sub>4</sub>

Внутри блоков кода (`` `…` `` и `` ```…``` ``) нельзя использовать HTML-теги, но ведь можно и сами блоки кода создавать с помощью HTML-тегов:
* `<code>a &minus; b</code>` выглядит как <code>a &minus; b</code>
* `<pre><code>a &minus; b</code></pre>` выглядит как <pre><code>a &minus; b</code></pre>

Источник: https://daringfireball.net/projects/markdown/syntax
