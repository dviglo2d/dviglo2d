// Деление путём подбора разрядов

#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


constexpr uint32_t base = 1'000'000'000;
using Digit = uint32_t;
using DDigit = uint64_t;
constexpr size_t chunk_length = 9;

// Убеждаемся, что bool всегда преобразуется в 1 или 0
static_assert(uint32_t(5 > 3) == 1 && uint32_t(bool(7)) == 1);

// Вычитает два положительных длинных числа столбиком.
// Уменьшаемое minuend должно быть >= вычитаемого subtrahend.
// Функция скопирована из 1_basics_sub_2.cpp
vector<Digit> sub(const vector<Digit>& minuend, const vector<Digit>& subtrahend)
{
    vector<Digit> ret;
    ret.resize(minuend.size());

    Digit borrow = 0;

    for (size_t i = 0; i < subtrahend.size(); ++i)
    {
        Digit minuend_digit = minuend[i];
        Digit subtrahend_digit = subtrahend[i] + borrow;
        borrow = minuend_digit < subtrahend_digit;
        assert(borrow == 0 || borrow == 1);
        minuend_digit += base * borrow;
        ret[i] = minuend_digit - subtrahend_digit;
    }

    for (size_t i = subtrahend.size(); i < minuend.size(); ++i)
    {
        Digit minuend_digit = minuend[i];
        Digit subtrahend_digit = borrow;
        borrow = minuend_digit < subtrahend_digit;
        assert(borrow == 0 || borrow == 1);
        minuend_digit += base * borrow;
        ret[i] = minuend_digit - subtrahend_digit;
    }

    assert(borrow == 0);

    while (ret.size() > 1 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Перемножает два положительных длинных числа столбиком.
// Функция скопирована из 1_basics_mul_2.cpp
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
// Функция скопирована из 1_basics_mul_2.cpp
string to_string(const vector<Digit>& big_num)
{
    assert(big_num.size() > 0);

    string ret = std::to_string(big_num.back());

    for (size_t i = big_num.size() - 2; i != size_t(-1); --i)
        ret += format("{0:0>{1}}", big_num[i], chunk_length);

    return ret;
}

// Преобразует строку в длинное число.
// Функция скопирована из 1_basics_mul_2.cpp
vector<Digit> to_num(const string& str)
{
    if (str.empty())
        return vector<Digit>{0};

    size_t num_chunks = str.length() / chunk_length;
    size_t rest_length = str.length() - num_chunks * chunk_length;
    size_t first_chunk_length;

    if (!rest_length)
    {
        first_chunk_length = chunk_length;
    }
    else
    {
        first_chunk_length = rest_length;
        ++num_chunks;
    }

    vector<Digit> ret;

    for (size_t i = 0; i < num_chunks - 1; ++i)
    {
        size_t chunk_start = str.size() - (i + 1) * chunk_length;
        string chunk = str.substr(chunk_start, chunk_length);
        ret.push_back(stoi(chunk));
    }

    string first_chunk = str.substr(0, first_chunk_length);
    ret.push_back(stoi(first_chunk));

    while (ret.size() > 1 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Определяет, меньше ли первое число.
// Функция скопирована из 1_basics_div.cpp
bool first_is_less(const vector<Digit>& first, const vector<Digit>& second)
{
    assert(first.size() == 1 || first.back() != 0);
    assert(second.size() == 1 || second.back() != 0);

    if (first.size() != second.size())
        return first.size() < second.size();

    for (size_t i = first.size() - 1; i != size_t(-1); --i)
    {
        if (first[i] != second[i])
            return first[i] < second[i];
    }

    return false;
}

// Определяет, что длинное число равно нулю.
// Функция скопирована из 1_basics_div.cpp
bool is_zero(const vector<Digit>& number)
{
    return (number.size() == 1 && number[0] == 0);
}

// Возвращает неполное частное и остаток.
// Если знаменатель == 0, возвращает {0, 0}.
// Функция изменена по сравнению с 1_basics_div.cpp (подбираем разряды, а не вычитаем в цикле)
pair<vector<Digit>, vector<Digit>> div_mod(const vector<Digit>& numerator, const vector<Digit>& denominator)
{
    // Если числитель или знаменатель == 0
    if (is_zero(numerator) || is_zero(denominator))
        return {vector<Digit>{0}, vector<Digit>{0}};

    // Если числитель меньше знаменателя
    if (first_is_less(numerator, denominator))
        return {vector<Digit>{0}, numerator};

    // Неполное частное
    vector<Digit> quotient;
    quotient.resize(numerator.size() - denominator.size() + 1, 0);

    // Подбираем разряды, начиная со старшего разряда (с конца)
    for (size_t i = quotient.size() - 1; i != size_t(-1); --i)
    {
        // Пока quotient * denominator <= numerator
        while (!first_is_less(numerator, mul(quotient, denominator))) // Ведущие нули убираются при умножении
            ++quotient[i]; // Увеличиваем разряд

        assert(quotient[i] > 0);
        --quotient[i]; // Откатываем разряд на один шаг назад
    }

    // Убираем ведущие нули
    while (quotient.size() > 1 && quotient.back() == 0)
        quotient.pop_back();

    // Остаток = numerator - quotient * denominator
    vector<Digit> remainder = sub(numerator, mul(quotient, denominator));

    return {quotient, remainder};
}

// Печатает два длинных числа и результат их деления
void show(const vector<Digit>& a, const vector<Digit>& b)
{
    pair<vector<Digit>, vector<Digit>> result = div_mod(a, b);

    cout << to_string(a) << " / " << to_string(b) << " = " << to_string(result.first)
         << " (remainder = " << to_string(result.second) << ")" << endl;
}

int main()
{
    show(to_num("0"), to_num("0"));      // 0, 0
    show(to_num("5"), to_num("3"));      // 1, 2
    show(to_num("123"), to_num("67"));   // 1, 56
    show(to_num("512"), to_num("17"));   // 30, 2
    show(to_num("999"), to_num("999"));  // 1, 0
    show(to_num("9999"), to_num("000")); // 0, 0

    // 566, 994340
    show(to_num("567000000"), to_num("1000010"));

    // 5000, 1111
    show(to_num("11111111111111111111111111111111111111"), to_num("2222222222222222222222222222222222"));

    // 130686, 5304519702476588520600956014
    show(to_num("1293564533453453419234546456456456"), to_num("9898223443473294328742373747"));

    return 0;
}
