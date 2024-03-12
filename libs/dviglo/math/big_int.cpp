// Copyright (c) the Dviglo project
// License: MIT

#include "big_int.hpp"

#include <cassert>

using namespace std;


namespace dviglo
{

#if true
    static constexpr BigInt::Digit base = 1000'000'000;

    /// Число десятичных цифр в каждой цифре Digit при преобразовании в строку
    static constexpr i32 base_digits = 9;
#else // Для тестов
    static constexpr BigInt::Digit base = 10;
    static constexpr i32 base_digits = 1;
#endif

// Сравнивает модули
static bool first_is_less(const vector<BigInt::Digit>& first, const vector<BigInt::Digit>& second)
{
    if (first.size() != second.size())
        return first.size() < second.size();

    for (i32 i = (i32)first.size() - 1; i >= 0; --i)
    {
        if (first[i] != second[i])
            return first[i] < second[i];
    }

    return false; // first == second
}

// Складывает модули столбиком
static vector<BigInt::Digit> sum_magnitudes(const vector<BigInt::Digit>& a, const vector<BigInt::Digit>& b)
{
    vector<BigInt::Digit> ret;
    size_t max_size = max(a.size(), b.size());
    ret.resize(max_size);

    // Перенос в следующий столбец
    BigInt::Digit carry = 0; // Всегда 0 или 1

    for (size_t i = 0; i < max_size; ++i)
    {
        BigInt::Digit a_element = i < a.size() ? a[i] : 0;
        BigInt::Digit b_element = i < b.size() ? b[i] : 0;
        BigInt::Digit sum = a_element + b_element + carry;
        carry = 0;

        if (sum >= base)
        {
            carry = 1;
            sum -= base;
        }

        ret[i] = sum;
    }

    if (carry)
        ret.push_back(1);

    return ret;
}

// Вычитает модули столбиком (a должен быть >= b)
static vector<BigInt::Digit> diff_magnitudes(const vector<BigInt::Digit>& a, const vector<BigInt::Digit>& b)
{
    vector<BigInt::Digit> ret;
    ret.resize(a.size());

    // Заём из следующего столбца
    BigInt::Digit borrow = 0; // Всегда 0 или 1

    for (size_t i = 0; i < a.size(); ++i)
    {
        BigInt::Digit a_element = a[i];
        BigInt::Digit b_element = i < b.size() ? b[i] : 0;

        // Если занимали, то на данном шаге нужно вычесть на 1 больше
        b_element += borrow;
        borrow = 0;

        if (a_element < b_element)
        {
            a_element += base;
            borrow = 1;
        }

        ret[i] = a_element - b_element;
    }

    assert(borrow == 0);

    // Убираем ведущие нули
    while (ret.size() >= 2 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Возвращает неполное частное и остаток.
// Если знаменатель == 0, возвращает {0, 0}
pair<BigInt, BigInt> div_mod(BigInt numerator, BigInt denominator)
{
    if (denominator == 0)
        return {0, 0};

    BigInt abs_num = abs(numerator);
    BigInt abs_denom = abs(denominator);

    BigInt quotient = 0;
    BigInt remainder = abs_num;

    // Простейший (и медленный) способ
    while (remainder >= abs_denom)
    {
        ++quotient;
        remainder -= abs_denom;
    }

    // https://en.cppreference.com/w/cpp/language/operator_arithmetic
    // (a/b)*b + a%b == a
    // 7/3 = {2, 1}    | 2*3 + 1 == 7
    // -7/3 = {-2, -1} | -2*3 + -1 == -7
    // 7/-3 = {-2, 1}  | -2*-3 + 1 == 7
    // -7/-3 = {2, -1} | 2*-3 + -1 == -7

    if (numerator.is_positive() != denominator.is_positive())
        quotient = -quotient;

    if (numerator.is_negative())
        remainder = -remainder;

    return {quotient, remainder};
}

BigInt::BigInt()
{
    positive_ = true;
    magnitude_.push_back(0);
}

BigInt::BigInt(i32 value)
{
    positive_ = (value >= 0);

#if false
    // abs((i32)-2147483648) возвращает -2147483648. Чтобы это пофиксить, сперва преобразуем результат в u32
    u64 val = (u32)std::abs(value);
    // Этот код вызывает баг в релизной сборке GCC 13 в тесте
    // assert(BigInt((i32)-0x80000000).to_string() == "-2147483648");
    // В отладочной сборке и в других компиляторах всё нормально
#else
    u64 val = (u64)std::abs((i64)value);
#endif

    while (val != 0)
    {
        Digit mod = val % base;
        magnitude_.push_back(mod);
        val /= base;
    }

    if (!magnitude_.size()) // value == 0
        magnitude_.push_back(0);
}

BigInt::BigInt(u32 value)
{
    positive_ = true;

    while (value != 0)
    {
        Digit mod = value % base;
        magnitude_.push_back(mod);
        value /= base;
    }

    if (!magnitude_.size()) // value == 0
        magnitude_.push_back(0);
}

BigInt::BigInt(i64 value)
{
    positive_ = (value >= 0);

    // abs((i64)-9223372036854775808) возвращает -9223372036854775808. Чтобы это пофиксить, преобразуем в u64
    u64 val = (u64)std::abs(value);

    while (val != 0)
    {
        Digit mod = val % base;
        magnitude_.push_back(mod);
        val /= base;
    }

    if (!magnitude_.size()) // value == 0
        magnitude_.push_back(0);
}

BigInt::BigInt(u64 value)
{
    positive_ = true;

    while (value != 0)
    {
        Digit mod = (Digit)(value % base);
        magnitude_.push_back(mod);
        value /= base;
    }

    if (!magnitude_.size()) // value == 0
        magnitude_.push_back(0);
}

BigInt::BigInt(const StrUtf8& str)
{
    if (str.empty())
    {
        // Инцициализируем нулём
        positive_ = true;
        magnitude_.push_back(0);
        return;
    }

    i32 first_digit_pos;

    if (str[0] == '-')
    {
        positive_ = false;
        first_digit_pos = 1;
    }
    else if (str[0] == '+')
    {
        positive_ = true;
        first_digit_pos = 1;
    }
    else
    {
        positive_ = true;
        first_digit_pos = 0;
    }

    // Пропускаем ведущие нули
    for (i32 i = first_digit_pos; i < (i32)str.length(); ++i)
    {
        if (str[i] != '0')
            break;

        ++first_digit_pos;
    }

    i32 last_digit_pos = -1;

    for (i32 i = first_digit_pos; i < (i32)str.length(); ++i)
    {
        if (!isdigit(str[i]))
            break;

        last_digit_pos = i;
    }

    // Если нет ни одной цифры
    if (last_digit_pos == -1)
    {
        // Инцициализируем нулём
        positive_ = true;
        magnitude_.push_back(0);
        return;
    }

    i32 num_digits = last_digit_pos - first_digit_pos + 1;
    magnitude_.reserve(num_digits / base_digits + 1);

    // В обратном порядке извлекаем куски по 9 цифр и преобразуем в Digit
    i32 i = last_digit_pos - base_digits + 1;
    for (; i > first_digit_pos; i -= base_digits)
    {
        StrUtf8 chunk = str.substr(i, base_digits);
        magnitude_.push_back(to_u64(chunk));
    }

    StrUtf8 last_chunk = str.substr(first_digit_pos, base_digits + i - first_digit_pos);
    magnitude_.push_back(to_u64(last_chunk));
}

bool BigInt::is_zero() const
{
    assert(magnitude_.size() > 0);

    if (magnitude_.size() == 1 && magnitude_[0] == 0)
    {
        assert(positive_);
        return true;
    }

    return false;
}

strong_ordering BigInt::operator<=>(const BigInt& rhs) const
{
    // Если у чисел разные знаки
    if (positive_ != rhs.positive_)
    {
        // То, положительное больше отрицательного
        return positive_ ? strong_ordering::greater : strong_ordering::less;
    }

    // В этой точке у чисел одинаковый знак

    // Если у чисел разная длина
    if (magnitude_.size() != rhs.magnitude_.size())
    {
        // Если числа положительные
        if (positive_)
        {
            // То более длинное число больше
            return (magnitude_.size() > rhs.magnitude_.size()) ? strong_ordering::greater : strong_ordering::less;
        }
        else // А если числа отрицательные
        {
            // То более короткое число больше
            return (magnitude_.size() < rhs.magnitude_.size()) ? strong_ordering::greater : strong_ordering::less;
        }
    }

    // В этой точке у чисел одинаковая длина

    // Сравниваем цифры в обратном порядке
    for (i32 i = (i32)magnitude_.size() - 1; i >= 0; --i)
    {
        if (magnitude_[i] != rhs.magnitude_[i])
        {
            if (positive_)
                return (magnitude_[i] > rhs.magnitude_[i]) ? strong_ordering::greater : strong_ordering::less;
            else
                return (magnitude_[i] < rhs.magnitude_[i]) ? strong_ordering::greater : strong_ordering::less;
        }
    }

    return strong_ordering::equal;
}

BigInt BigInt::operator+(const BigInt& rhs) const
{
    BigInt ret;

    if (positive_ == rhs.positive_)
    {
        ret.positive_ = positive_;
        ret.magnitude_ = sum_magnitudes(magnitude_, rhs.magnitude_);
    }
    else
    {
        if (first_is_less(magnitude_, rhs.magnitude_))
        {
            ret.positive_ = rhs.positive_;
            ret.magnitude_ = diff_magnitudes(rhs.magnitude_, magnitude_);
        }
        else
        {
            ret.positive_ = positive_;
            ret.magnitude_ = diff_magnitudes(magnitude_, rhs.magnitude_);
        }
    }

    return ret;
}


BigInt BigInt::operator-(const BigInt& rhs) const
{
    BigInt ret;

    if (positive_ != rhs.positive_)
    {
        ret.positive_ = positive_;
        ret.magnitude_ = sum_magnitudes(magnitude_, rhs.magnitude_);
    }
    else
    {
        if (first_is_less(magnitude_, rhs.magnitude_))
        {
            ret.positive_ = !rhs.positive_;
            ret.magnitude_ = diff_magnitudes(rhs.magnitude_, magnitude_);
        }
        else
        {
            ret.positive_ = positive_;
            ret.magnitude_ = diff_magnitudes(magnitude_, rhs.magnitude_);
        }
    }

    return ret;
}

// Простое умножение столбиком
BigInt BigInt::operator*(const BigInt& rhs) const
{
    BigInt ret;
    ret.magnitude_.resize(magnitude_.size() + rhs.magnitude_.size(), 0);

    if (positive_ == rhs.positive_)
        ret.positive_ = true;
    else
        ret.positive_ = false;

    for (i32 this_index = 0; this_index < magnitude_.size(); ++this_index)
    {
        for (i32 rhs_index = 0; rhs_index < rhs.magnitude_.size(); ++rhs_index)
            ret.magnitude_[this_index + rhs_index] += magnitude_[this_index] * rhs.magnitude_[rhs_index];
    }

    for (i32 i = 0; i < ret.magnitude_.size() - 1; ++i)
    {
        ret.magnitude_[i + 1] += ret.magnitude_[i] / base;
        ret.magnitude_[i] %= base;
    }

    // Удаляем ведущие нули
    while (ret.magnitude_.size() >= 2 && ret.magnitude_.back() == 0)
        ret.magnitude_.pop_back();

    return ret;
}

BigInt BigInt::operator/(const BigInt& rhs) const
{
    pair<BigInt, BigInt> dm = div_mod(*this, rhs);
    return dm.first;
}

BigInt BigInt::operator%(const BigInt& rhs) const
{
    pair<BigInt, BigInt> dm = div_mod(*this, rhs);
    return dm.second;
}

BigInt BigInt::operator-() const
{
    BigInt ret = *this;
    if (!ret.is_zero())
        ret.positive_ = !ret.positive_;
    return ret;
}

BigInt& BigInt::operator+=(const BigInt& rhs)
{
    BigInt result = *this + rhs;
    swap(this->positive_, result.positive_);
    swap(this->magnitude_, result.magnitude_);
    return *this;
}

BigInt& BigInt::operator-=(const BigInt& rhs)
{
    BigInt result = *this - rhs;
    swap(this->positive_, result.positive_);
    swap(this->magnitude_, result.magnitude_);
    return *this;
}

BigInt& BigInt::operator*=(const BigInt& rhs)
{
    BigInt result = *this * rhs;
    swap(this->positive_, result.positive_);
    swap(this->magnitude_, result.magnitude_);
    return *this;
}

BigInt& BigInt::operator/=(const BigInt& rhs)
{
    BigInt result = *this / rhs;
    swap(this->positive_, result.positive_);
    swap(this->magnitude_, result.magnitude_);
    return *this;
}

BigInt& BigInt::operator%=(const BigInt& rhs)
{
    BigInt result = *this % rhs;
    swap(this->positive_, result.positive_);
    swap(this->magnitude_, result.magnitude_);
    return *this;
}

StrUtf8 BigInt::to_string() const
{
    assert(magnitude_.size() > 0);

    StrUtf8 ret;

    if (!positive_)
        ret = "-";

    // Первый кусок результата без ведущих нулей
    i32 i = (i32)magnitude_.size() - 1;
    ret += std::to_string(magnitude_[i]);
    --i;

    // Остальные куски результата
    for (; i >= 0; --i)
    {
        StrUtf8 str = std::to_string(magnitude_[i]);

        // Ведущие нули куска
        if (str.length() < base_digits)
        {
            StrUtf8 zeros(base_digits - str.length(), '0');
            str = zeros + str;
        }

        ret += str;
    }

    return ret;
}

} // namespace dviglo
