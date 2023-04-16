// Сложение столбиком

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


// Отдельная цифра длинного числа
using Digit = uint32_t;

// Складывает два положительных длинных десятичных числа столбиком
vector<Digit> add(const vector<Digit>& a, const vector<Digit>& b)
{
    // Выясняем, какое число длиннее
    const vector<Digit>& long_num = a.size() >= b.size() ? a : b;
    const vector<Digit>& short_num = a.size() >= b.size() ? b : a;

    vector<Digit> ret; // Результат

    // Насколько число short_num короче числа long_num
    size_t len_diff = long_num.size() - short_num.size();

    // Перенос в старший разряд (всегда 0 или 1)
    Digit carry = 0;

    // Цикл начинается справа (с младших разрядов)
    for (size_t index_short = short_num.size() - 1; index_short != size_t(-1); --index_short)
    {
        // Разряды, которые нужно складывать, имеют разные индексы (из-за разной длины чисел)
        size_t index_long = index_short + len_diff;

        uint32_t sum = long_num[index_long] + short_num[index_short] + carry;
        carry = sum / 10;
        assert(carry == 0 || carry == 1);
        ret.insert(ret.begin(), sum % 10); // Добавляем цифру слева
    }

    // Оставшиеся цифры более длинного числа
    for (size_t index_long = len_diff - 1; index_long != size_t(-1); --index_long)
    {
        uint32_t sum = long_num[index_long] + carry;
        carry = sum / 10;
        assert(carry == 0 || carry == 1);
        ret.insert(ret.begin(), sum % 10); // Добавляем цифру слева
    }

    if (carry)
        ret.insert(ret.begin(), 1);

    return ret;
}

// Преобразует длинное число в строку
string to_string(const vector<Digit>& big_num)
{
    string ret;

    for (Digit digit : big_num)
        ret += std::to_string(digit);

    return ret;
}

// Печатает два длинных числа и их сумму
void show(const vector<Digit>& a, const vector<Digit>& b)
{
    cout << to_string(a) << " + " << to_string(b) << " = " << to_string(add(a, b)) << endl;
}

int main()
{
    show({1, 2, 3}, {4, 5, 6});       // 123 + 456 = 579
    show({9, 9, 9}, {1, 2});          // 999 + 12 = 1011
    show({1, 2}, {9, 9, 9});          // 12 + 999 = 1011
    show({1, 2, 3}, {9, 9, 9});       // 123 + 999 = 1122
    show({9, 9, 9, 9}, {9, 9, 9, 9}); // 9999 + 9999 = 19998

    return 0;
}
