// Вычитание столбиком

#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


constexpr uint32_t base = 1'000'000'000;
using Digit = uint32_t;
constexpr size_t chunk_length = 9;

// Вычитает два положительных длинных числа столбиком.
// Уменьшаемое minuend должно быть >= вычитаемого subtrahend
vector<Digit> sub(const vector<Digit>& minuend, const vector<Digit>& subtrahend)
{
    vector<Digit> ret;
    ret.resize(minuend.size());

    // Заём из следующего столбца
    Digit borrow = 0; // Всегда 0 или 1

    // Вычитаем цифры чисел, начиная с младших разрядов
    for (size_t i = 0; i < subtrahend.size(); ++i)
    {
        Digit minuend_digit = minuend[i];

        // Если занимали на предыдущем шаге, то нужно вычесть на 1 больше
        Digit subtrahend_digit = subtrahend[i] + borrow;
        borrow = 0;

        // Если нужен заём на данном шаге
        if (minuend_digit < subtrahend_digit)
        {
            // Занимаем из старшего разряда
            minuend_digit += base;
            borrow = 1;
        }

        ret[i] = minuend_digit - subtrahend_digit;
    }

    // Оставшиеся цифры minuend
    for (size_t i = subtrahend.size(); i < minuend.size(); ++i)
    {
        Digit minuend_digit = minuend[i];
        Digit subtrahend_digit = borrow;
        borrow = 0;

        if (minuend_digit < subtrahend_digit)
        {
            minuend_digit += base;
            borrow = 1;
        }

        ret[i] = minuend_digit - subtrahend_digit;
    }

    assert(borrow == 0);

    // Убираем ведущие нули
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

// Печатает два длинных числа и их разность
void show(const vector<Digit>& a, const vector<Digit>& b)
{
    cout << to_string(a) << " - " << to_string(b) << " = " << to_string(sub(a, b)) << endl;
}

int main()
{
    show(to_num("0"), to_num("0"));      // 0
    show(to_num("5"), to_num("3"));      // 2
    show(to_num("123"), to_num("67"));   // 56
    show(to_num("512"), to_num("17"));   // 495
    show(to_num("999"), to_num("999"));  // 0
    show(to_num("9999"), to_num("000")); // 9999

    // 565999990
    show(to_num("567000000"), to_num("1000010"));

    // 11111111111111111108888888888888888889
    show(to_num("11111111111111111111111111111111111111"), to_num("2222222222222222222"));

    // 1293563543631109071905113582219082
    show(to_num("1293564533453453419234546456456456"), to_num("989822344347329432874237374"));

    // 9990000000000000000000000000000000
    show(to_num("9999999999999999999999999999999999"), to_num("9999999999999999999999999999999"));

    return 0;
}
