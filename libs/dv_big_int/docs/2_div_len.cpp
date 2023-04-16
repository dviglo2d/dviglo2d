// Точная длина неполного частного

#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <vector>

using namespace std;


using Digit = uint32_t;

#if 1
constexpr uint32_t base = 1'0;
constexpr size_t chunk_length = 1;
#elif 1
constexpr uint32_t base = 1'00;
constexpr size_t chunk_length = 2;
#elif 1
constexpr uint32_t base = 1'000'000'000;
constexpr size_t chunk_length = 9;
#endif

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

// Определяет длину неполного частного
size_t calc_quotient(const vector<Digit>& numerator, const vector<Digit>& denominator)
{
    if (is_zero(denominator))
        return 1; // Пусть при делении на 0 результатом будет 0 (длина = 1)

    if (first_is_less(numerator, denominator)) // Если числитель меньше знаменателя
        return 1; // Неполное частное = 0 (длина = 1)

    vector<Digit> chunk; // Первое неполное делимое

    // Ищем первое неполное делимое (начинаем со старших разрядов)
    for (size_t i = numerator.size() - 1; i != size_t(-1); --i)
    {
        // Добавляем цифру слева (так как храним цифры в обратном порядке)
        chunk.insert(chunk.begin(), numerator[i]);

        if (!first_is_less(chunk, denominator)) // chunk >= denominator
            break; // Нашли первое неполное делимое
    }

    // Неполное делимое максимум на одну цифру длиннее знаменателя
    assert(chunk.size() == denominator.size() || chunk.size() == denominator.size() + 1);

    size_t ret = numerator.size() - chunk.size() + 1;
    assert(ret == numerator.size() - denominator.size() || ret == numerator.size() - denominator.size() + 1);

    return ret;
}

// Печатает два длинных числа и длину неполного частного
void show(const vector<Digit>& a, const vector<Digit>& b)
{
    cout << to_string(a) << " / " << to_string(b) << " | result length = " << calc_quotient(a, b) << endl;
}

int main()
{
    show(to_num("0"), to_num("0"));      // 1 (0)
    show(to_num("5"), to_num("3"));      // 1 (1)
    show(to_num("123"), to_num("67"));   // 1 (1)
    show(to_num("512"), to_num("17"));   // 2 (30)
    show(to_num("999"), to_num("999"));  // 1 (1)
    show(to_num("9999"), to_num("000")); // 1 (0)

    // 3 (результат деления = 566)
    show(to_num("567000000"), to_num("1000010"));

    // 4 (5000)
    show(to_num("11111111111111111111111111111111111111"), to_num("2222222222222222222222222222222222"));

    // 6 (130686)
    show(to_num("1293564533453453419234546456456456"), to_num("9898223443473294328742373747"));

    // 50 (13068658137053464352525468785867118980456379357846)
    show(to_num("12935645334534534192345464564564563443473294328742373747"), to_num("989822"));

    // 10 (3000000000)
    show(to_num("15000000000"), to_num("5"));

    return 0;
}
