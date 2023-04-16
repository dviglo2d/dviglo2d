// Умножение столбиком: смена base

#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


// Основание системы счисления
constexpr uint32_t base = 1'000'000'000;

// Отдельная цифра длинного числа (0, 1, 2, ..., base-1)
using Digit = uint32_t;

// Тип удвоенной (double) длины. Умещает квадрат цифры
using DDigit = uint64_t;

// Число десятичных цифр в каждой цифре Digit при преобразовании в строку и обратно
constexpr size_t chunk_length = 9;

// Перемножает два положительных длинных числа столбиком.
// Функция изменена по сравнению с 1_basics_mul_1.cpp (сменили base)
vector<Digit> mul(const vector<Digit>& a, const vector<Digit>& b)
{
    assert(a.size() > 0 && b.size() > 0);

    vector<Digit> ret;
    ret.resize(a.size() + b.size(), 0);

    for (size_t a_index = 0; a_index < a.size(); ++a_index)
    {
        Digit carry = 0;

        for (size_t b_index = 0; b_index < b.size(); ++b_index)
        {
            size_t ret_index = a_index + b_index;

            // uint32_t не может уместить base^2 - 1, а uint64_t - может
            DDigit v = ret[ret_index] + (DDigit)a[a_index] * b[b_index] + carry;
            assert(v <= (DDigit)base * base - 1);

            carry = Digit(v / base);
            assert(carry < base);
            ret[ret_index] = v % base;
        }

        assert(ret[a_index + b.size()] == 0);
        ret[a_index + b.size()] = carry;
    }

    while (ret.size() > 1 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Преобразует длинное число в строку.
// Функция изменена по сравнению с 1_basics_mul_1.cpp
// (теперь одна цифра длинного числа соответствует девяти символам строки, а не одному)
string to_string(const vector<Digit>& big_num)
{
    assert(big_num.size() > 0);

    // Первый кусок результата без ведущих нулей
    string ret = std::to_string(big_num.back());

    // Остальные куски результата с ведущими нулями
    for (size_t i = big_num.size() - 2; i != size_t(-1); --i)
        ret += format("{0:0>{1}}", big_num[i], chunk_length);

    return ret;
}

// Преобразует строку в длинное число.
// Функция изменена по сравнению с 1_basics_mul_1.cpp
// (теперь одна цифра длинного числа соответствует девяти символам строки, а не одному)
vector<Digit> to_num(const string& str)
{
    if (str.empty())
        return vector<Digit>{0};

    size_t num_chunks = str.length() / chunk_length; // Число кусков в строке
    size_t rest_length = str.length() - num_chunks * chunk_length;
    size_t first_chunk_length;

    if (!rest_length) // Первый кусок полный
    {
        first_chunk_length = chunk_length;
    }
    else // Первый кусок неполный
    {
        first_chunk_length = rest_length;
        ++num_chunks;
    }

    vector<Digit> ret;

    // Извлекаем куски в обратном порядке и преобразуем в Digit (кроме первого куска)
    for (size_t i = 0; i < num_chunks - 1; ++i) // i - номер куска с конца
    {
        size_t chunk_start = str.size() - (i + 1) * chunk_length;
        string chunk = str.substr(chunk_start, chunk_length);
        ret.push_back(stoi(chunk));
    }

    // Преобразуем в Digit первый кусок
    string first_chunk = str.substr(0, first_chunk_length);
    ret.push_back(stoi(first_chunk));

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
