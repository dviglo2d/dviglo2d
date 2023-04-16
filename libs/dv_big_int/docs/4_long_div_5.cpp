// Деление столбиком: уменьшаем шанс промаха

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

// Без угадываний делит длинное число на цифру (столбиком).
// Если знаменатель == 0, возвращает {0, 0}.
// Функция скопирована из 4_long_div_2.cpp
static pair<vector<Digit>, vector<Digit>> div_by_digit(const vector<Digit>& numerator, Digit denominator)
{
    if (is_zero(numerator) || denominator == 0)
        return {vector<Digit>{0}, vector<Digit>{0}};

    if (first_is_less(numerator, vector<Digit>{denominator}))
        return {vector<Digit>{0}, numerator};

    vector<Digit> quotient;
    DDigit chunk = 0;

    for (size_t i = numerator.size() - 1; i != size_t(-1); --i)
    {
        chunk = chunk * base + numerator[i];
        assert(chunk / denominator < base);
        Digit digit = Digit(chunk / denominator);
        quotient.push_back(digit);
        chunk -= (DDigit)digit * denominator;
        assert(chunk < base);
    }

    reverse(quotient.begin(), quotient.end());

    while (quotient.size() > 1 && quotient.back() == 0)
        quotient.pop_back();

    return {quotient, {(Digit)chunk}};
}

// Отбрасывает младший разряд.
// Функция скопирована из 4_long_div_1.cpp
vector<Digit> div_by_base(const vector<Digit>& number)
{
    if (number.size() == 1)
        return vector<Digit>{0};

    return vector<Digit>(number.cbegin() + 1, number.cend());
}

// Делит кусок числителя на знаменатель.
// Кусок и знаменатель таковы, что в результате всегда одна цифра.
// К тому же аргументы должны быть нормализованы (старшая цифра знаменателя >= base / 2).
// Функция изменена по сравнению с 4_long_div_4.cpp
// (изучаем ещё по одной цифре куска и знаменателя)
Digit div_chunk(const vector<Digit>& chunk, const vector<Digit>& denominator)
{
    if (first_is_less(chunk, denominator))
        return 0;

    assert(chunk.size() == denominator.size()
           || (chunk.size() == denominator.size() + 1
               && first_is_less(div_by_base(chunk), denominator)));

    DDigit denominator_digit_1 = denominator.back();
    assert(denominator_digit_1 >= base / 2);

    DDigit chunk_2_digits = chunk.back();
    if (chunk.size() != denominator.size())
        chunk_2_digits = chunk_2_digits * base + chunk[chunk.size() - 2];

    // Третья цифра куска
    DDigit chunk_digit_3 = (chunk.size() == denominator.size())
        ? chunk[chunk.size() - 2] // Старшая цифра куска = 0, но её нет в массиве
        : chunk[chunk.size() - 3];

    // Вторая цифра знаменателя
    DDigit denominator_digit_2 = denominator[denominator.size() - 2];

    // Делим две старшие цифры куска на старшую цифру знаменателя
    DDigit digit = chunk_2_digits / denominator_digit_1;

    // Остаток
    DDigit remainder = chunk_2_digits - digit * denominator_digit_1; // chunk_2_digits % denominator_digit_1

    // Уменьшаем цифру, если она точно слишком большая
    if (digit == base || digit * denominator_digit_2 > remainder * base + chunk_digit_3)
    {
        --digit;
        remainder += denominator_digit_1;

        if (remainder < base && (digit == base || digit * denominator_digit_2 > remainder * base + chunk_digit_3))
        {
            --digit;

#ifndef NDEBUG
            remainder += denominator_digit_1;
#endif
        }
    }

    assert(digit < base);
    assert(digit * denominator_digit_2 <= remainder * base + chunk_digit_3);

    // Теперь проверяем, нет ли промаха на 1
    if (first_is_less(chunk, mul(vector<Digit>{(Digit)digit}, denominator)))
        --digit;

    assert(!first_is_less(chunk, mul(vector<Digit>{(Digit)digit}, denominator)));

    return (Digit)digit;
}

// Возвращает неполное частное и остаток.
// Если знаменатель == 0, возвращает {0, 0}.
// Функция скопирована из 4_long_div_4.cpp
pair<vector<Digit>, vector<Digit>> div_mod(const vector<Digit>& numerator, const vector<Digit>& denominator)
{
    if (denominator.size() == 1)
        return div_by_digit(numerator, denominator[0]);

    if (is_zero(numerator) || is_zero(denominator))
        return {vector<Digit>{0}, vector<Digit>{0}};

    if (first_is_less(numerator, denominator))
        return {vector<Digit>{0}, numerator};

    Digit scale = base / (denominator.back() + 1);
    assert(scale <= base / 2);

    vector<Digit> norm_numerator = (scale == 1)
        ? numerator
        : mul(vector<Digit>{scale}, numerator);

    vector<Digit> norm_denominator = (scale == 1)
        ? denominator
        : mul(vector<Digit>{scale}, denominator);

    vector<Digit> quotient;
    vector<Digit> chunk;

    for (size_t i = norm_numerator.size() - 1; i != size_t(-1); --i)
    {
        chunk.insert(chunk.begin(), norm_numerator[i]);

        while (chunk.size() > 1 && chunk.back() == 0)
            chunk.pop_back();

        Digit digit = div_chunk(chunk, norm_denominator);
        quotient.push_back(digit);
        chunk = sub(chunk, mul(vector<Digit>{digit}, norm_denominator));
    }

    reverse(quotient.begin(), quotient.end());

    while (quotient.size() > 1 && quotient.back() == 0)
        quotient.pop_back();

    if (scale != 1)
    {
        pair<vector<Digit>, vector<Digit>> denorm_chunk = div_by_digit(chunk, scale);
        assert(denorm_chunk.second == vector<Digit>{0});
        chunk = denorm_chunk.first;
    }

    return {quotient, chunk};
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

    // 13068658137053464352525468785867118980456379357846, 530335
    show(to_num("12935645334534534192345464564564563443473294328742373747"), to_num("989822"));

    // 3000000000, 0
    show(to_num("15000000000"), to_num("5"));

    // 999999997, 3600000000
    show(to_num("100000000600000000900000000"), to_num("100000000900000000"));

    // 600430312, 686904167853
    show(to_num("1873135157604149223893"), to_num("3119654553545"));

    return 0;
}
