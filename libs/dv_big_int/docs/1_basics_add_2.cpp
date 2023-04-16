// Сложение столбиком: обратный порядок цифр

#include <cassert>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

using namespace std;


// Отдельная цифра длинного числа
using Digit = uint32_t;

// Складывает два положительных длинных десятичных числа столбиком.
// Функция изменена по сравнению с 1_basics_add_1.cpp (теперь цифры в обратном порядке)
vector<Digit> add(const vector<Digit>& a, const vector<Digit>& b)
{
    // Выясняем, какое число длиннее
    const vector<Digit>& long_num = a.size() >= b.size() ? a : b;
    const vector<Digit>& short_num = a.size() >= b.size() ? b : a;

    vector<Digit> ret; // Результат

    // Перенос в старший разряд (всегда 0 или 1)
    Digit carry = 0;

    // Цикл начинается слева (с младших разрядов)
    for (size_t i = 0; i < short_num.size(); ++i)
    {
        uint32_t sum = long_num[i] + short_num[i] + carry;
        carry = sum / 10;
        assert(carry == 0 || carry == 1);
        ret.push_back(sum % 10); // Добавляем цифру в конец
    }

    // Оставшиеся цифры более длинного числа
    for (size_t i = short_num.size(); i < long_num.size(); ++i)
    {
        uint32_t sum = long_num[i] + carry;
        carry = sum / 10;
        assert(carry == 0 || carry == 1);
        ret.push_back(sum % 10);
    }

    if (carry)
        ret.push_back(1);

    return ret;
}

// Преобразует длинное число в строку.
// Функция изменена по сравнению с 1_basics_add_1.cpp (теперь цифры в обратном порядке)
string to_string(const vector<Digit>& big_num)
{
    string ret;

    // Цикл по цифрам числа в обратном порядке
    for (Digit digit : big_num | views::reverse)
        ret += std::to_string(digit);

    return ret;
}

// Печатает два длинных числа и их сумму
void show(const vector<Digit>& a, const vector<Digit>& b)
{
    cout << to_string(a) << " + " << to_string(b) << " = " << to_string(add(a, b)) << endl;
}

// Функция изменена по сравнению с 1_basics_add_1.cpp (теперь цифры в обратном порядке)
int main()
{
    show({3, 2, 1}, {6, 5, 4});       // 123 + 456 = 579
    show({9, 9, 9}, {2, 1});          // 999 + 12 = 1011
    show({2, 1}, {9, 9, 9});          // 12 + 999 = 1011
    show({3, 2, 1}, {9, 9, 9});       // 123 + 999 = 1122
    show({9, 9, 9, 9}, {9, 9, 9, 9}); // 9999 + 9999 = 19998

    return 0;
}
