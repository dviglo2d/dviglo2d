// Умножение столбиком

#include <cassert>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

using namespace std;


// Отдельная цифра длинного числа
using Digit = uint32_t;

// Перемножает два положительных длинных десятичных числа столбиком
vector<Digit> mul(const vector<Digit>& a, const vector<Digit>& b)
{
    // В векторах должна быть хотя бы одна цифра
    assert(a.size() > 0 && b.size() > 0);

    vector<Digit> ret; // Результат

    // Максимальная длина результата = a.size() + b.size() (когда 999... * 999...),
    // а минимальная - 1 (например при умножении на 0)
    ret.resize(a.size() + b.size(), 0);

    // Каждую цифру числа a умножаем на число b (начиная с младших разрядов)
    for (size_t a_index = 0; a_index < a.size(); ++a_index)
    {
        Digit carry = 0; // Перенос в старший разряд
        // Максимальный перенос при умножении двух цифр = 8 (при 9 * 9).
        // Так как одновременно выполняется сложение, то может добавляться перенос от сложения (максимум 1).
        // Таким образом carry может достигать 9 (например при 99 * 99)

        for (size_t b_index = 0; b_index < b.size(); ++b_index)
        {
            size_t ret_index = a_index + b_index;

            ret[ret_index] += a[a_index] * b[b_index] + carry;
            assert(ret[ret_index] <= 9 + 9 * 9 + 9);
            // Максимальное значение в каждой ячейке вектора =
            // (base - 1) + (base - 1)^2 + (base - 1) = base^2 - 1
            // (например при 999 * 999)

            carry = ret[ret_index] / 10;
            assert(carry <= 9);
            ret[ret_index] %= 10;
        }

        assert(ret[a_index + b.size()] == 0);
        ret[a_index + b.size()] = carry;
    }

    // Убираем ведущие нули
    while (ret.size() > 1 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Преобразует длинное число в строку.
// Функция скопирована из 1_basics_add_2.cpp
string to_string(const vector<Digit>& big_num)
{
    string ret;

    for (Digit digit : big_num | views::reverse)
        ret += std::to_string(digit);

    return ret;
}

// Преобразует строку в длинное число
vector<Digit> to_num(const string& str)
{
    if (str.empty())
        return vector<Digit>{0};

    vector<Digit> ret;

    // Цикл по символам строки в обратном порядке
    for (char c : str | views::reverse)
        ret.push_back(Digit(c - '0'));

    // Убираем ведущие нули
    while (ret.size() > 1 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Печатает два длинных числа и их произведение
void show(const vector<Digit>& a, const vector<Digit>& b)
{
    cout << to_string(a) << " * " << to_string(b) << " = " << to_string(mul(a, b)) << endl;
}

int main()
{
    show(to_num("2"), to_num("8"));     // 16
    show(to_num("123"), to_num("67"));  // 8241
    show(to_num("17"), to_num("512"));  // 8704
    show(to_num("99"), to_num("99"));   // 9801
    show(to_num("999"), to_num("999")); // 998001
    show(to_num("000"), to_num("999")); // 0
    show(to_num("3"), to_num("745"));   // 2235

    // 567005670000000
    show(to_num("1000010"), to_num("567000000"));

    // 1280399079067456714112168894673698375814428579386489438786544
    show(to_num("1293564533453453419234546456456456"), to_num("989822344347329432874237374"));

    // 99999999999999999999999999999989990000000000000000000000000000001
    show(to_num("9999999999999999999999999999999999"), to_num("9999999999999999999999999999999"));

    return 0;
}
