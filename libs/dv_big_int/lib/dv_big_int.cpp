// Copyright (c) the Dviglo project
// License: MIT

#include "dv_big_int.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <random>

using namespace std;


namespace dviglo
{

constexpr uint32_t base = BigInt::base;
using Digit = BigInt::Digit;

// Тип удвоенной (double) длины. Умещает квадрат цифры
using DDigit = uint64_t;

// Определяет, меньше ли первый модуль
static bool first_is_less(const vector<Digit>& first, const vector<Digit>& second)
{
    // Проверяем, что нет ведущих нулей
    assert(first.size() == 1 || first.back() != 0);
    assert(second.size() == 1 || second.back() != 0);

    // В модулях не должно быть ведущих нулей
    if (first.size() != second.size())
        return first.size() < second.size(); // Если модуль короче, значит он меньше

    // Сравниваем разряды, начиная с конца (со старших разрядов)
    for (size_t i = first.size() - 1; i != size_t(-1); --i)
    {
        if (first[i] != second[i])
            return first[i] < second[i];
    }

    return false; // first == second
}

// Складывает модули чисел столбиком
static vector<Digit> add_magnitudes(const vector<Digit>& a, const vector<Digit>& b)
{
    // Выясняем, какой модуль длиннее
    const vector<Digit>& long_mag = a.size() >= b.size() ? a : b;
    const vector<Digit>& short_mag = a.size() < b.size() ? a : b;

    vector<Digit> ret;

    // Длина результата равна long_mag.size() или long_mag.size() + 1
    ret.reserve(long_mag.size() + 1);

    // Перенос в следующий столбец
    uint32_t carry = 0; // Всегда 0 или 1

    // Складываем цифры модулей, начиная с младших разрядов
    for (size_t i = 0; i < short_mag.size(); ++i)
    {
        // uint32_t хватает для хранения 999'999'999 + 999'999'999 + 1
        uint32_t sum = long_mag[i] + short_mag[i] + carry;

        carry = sum / base;
        assert(carry == 0 || carry == 1);
        ret.push_back(sum % base);
    }

    // Оставшиеся цифры более длинного модуля
    for (size_t i = short_mag.size(); i < long_mag.size(); ++i)
    {
        uint32_t sum = long_mag[i] + carry;
        carry = sum / base;
        assert(carry == 0 || carry == 1);
        ret.push_back(sum % base);
    }

    if (carry)
        ret.push_back(1);

    assert(ret.size() == long_mag.size() || long_mag.size() + 1);

    return ret;
}

// Вычитает модули чисел столбиком. Уменьшаемое minuend должно быть >= вычитаемого subtrahend
static vector<Digit> sub_magnitudes(const vector<Digit>& minuend, const vector<Digit>& subtrahend)
{
    vector<Digit> ret;
    ret.resize(minuend.size(), 0);

    // Заём из следующего столбца
    uint32_t borrow = 0; // Всегда 0 или 1

    // Вычитаем цифры модулей, начиная с младших разрядов
    for (size_t i = 0; i < subtrahend.size(); ++i)
    {
        Digit minuend_digit = minuend[i];

        // Если занимали на предыдущем шаге, то нужно вычесть на 1 больше
        Digit subtrahend_digit = subtrahend[i] + borrow;

        // Определяем, нужен ли заём на данном шаге
        borrow = minuend_digit < subtrahend_digit;
        assert(borrow == 0 || borrow == 1);

        // Занимаем из старшего разряда, если нужно
        minuend_digit += base * borrow;

        ret[i] = minuend_digit - subtrahend_digit;
    }

    // Оставшиеся цифры minuend
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

    // Убираем ведущие нули
    while (ret.size() > 1 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Перемножает модули чисел столбиком.
// Смотрите "Умножение столбиком и смена base" в туторе
static vector<Digit> mul_magnitudes(const vector<Digit>& a, const vector<Digit>& b)
{
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

    // Убираем ведущие нули
    while (ret.size() > 1 && ret.back() == 0)
        ret.pop_back();

    return ret;
}

// Без угадываний делит модуль длинного числа на цифру (столбиком).
// Если знаменатель == 0, возвращает {0, 0}
static pair<vector<Digit>, vector<Digit>> div_by_digit(const vector<Digit>& numerator, Digit denominator)
{
    // Если числитель или знаменатель == 0
    if (numerator == vector<Digit>{0} || denominator == 0)
        return {vector<Digit>{0}, vector<Digit>{0}};

    // Если числитель меньше знаменателя
    if (first_is_less(numerator, vector<Digit>{denominator}))
        return {vector<Digit>{0}, numerator};

    // Неполное частное
    vector<Digit> quotient;
    quotient.reserve(numerator.size());

    // Текущий кусок числителя (и одновременно остаток)
    DDigit chunk = 0;

    // Извлекаем цифры числителя, начиная с конца (со старших разрядов)
    for (size_t i = numerator.size() - 1; i != size_t(-1); --i)
    {
        chunk = chunk * base + numerator[i];

        // Делим кусок на знаменатель и получаем очередную цифру результата
        assert(chunk / denominator < base);
        Digit digit = Digit(chunk / denominator);

        // Добавляем цифру к результату
        quotient.push_back(digit);

        // chunk %= denominator
        chunk -= (DDigit)digit * denominator;
        assert(chunk < base);
    }

    // Мы добавляли цифры в конец вектора, а не в начало, поэтому реверсируем
    reverse(quotient.begin(), quotient.end());

    // Убираем ведущие нули
    while (quotient.size() > 1 && quotient.back() == 0)
        quotient.pop_back();

    return {quotient, {(Digit)chunk}}; // В chunk находится остаток
}

#ifndef NDEBUG
// Отбрасывает младший разряд
static vector<Digit> div_by_base(const vector<Digit>& magnitude)
{
    if (magnitude.size() == 1) // Модуль < base
        return vector<Digit>{0};

    // Отбрасываем цифру слева
    return vector<Digit>(magnitude.cbegin() + 1, magnitude.cend());
}
#endif

// Делит кусок числителя на знаменатель.
// Кусок и знаменатель таковы, что в результате всегда одна цифра.
// К тому же аргументы должны быть нормализованы (старшая цифра знаменателя >= base / 2)
static Digit div_chunk(const vector<Digit>& chunk, const vector<Digit>& denominator)
{
    if (first_is_less(chunk, denominator))
        return 0;

    assert(chunk.size() == denominator.size() // Длина куска равна длине знаменателя
           || (chunk.size() == denominator.size() + 1 // Или кусок длиннее знаменателя на 1 цифру
               && first_is_less(div_by_base(chunk), denominator))); // И эта цифра не лишняя

    // Когда в знаменателе одна цифра, используется div_by_digit()
    assert(denominator.size() >= 2);

    // Старшая цифра знаменателя
    DDigit denominator_digit_1 = denominator.back();

    // Должно быть нормализовано
    assert(denominator_digit_1 >= base / 2);

    // Две старшие цифры куска.
    // Если длина куска равна длине знаменателя, то старшая цифра будет 0
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
    if (first_is_less(chunk, mul_magnitudes(vector<Digit>{(Digit)digit}, denominator)))
        --digit;

    assert(!first_is_less(chunk, mul_magnitudes(vector<Digit>{(Digit)digit}, denominator)));

    return (Digit)digit;
}

// Возвращает неполное частное и остаток (делит столбиком).
// Если знаменатель == 0, возвращает {0, 0}
static pair<vector<Digit>, vector<Digit>> div_mod_magnitudes(const vector<Digit>& numerator, const vector<Digit>& denominator)
{
    // Если в знаменателе одна цифра
    if (denominator.size() == 1)
        return div_by_digit(numerator, denominator[0]);

    // Если числитель или знаменатель == 0
    if (numerator == vector<Digit>{0} || denominator == vector<Digit>{0})
        return {vector<Digit>{0}, vector<Digit>{0}};

    // Если числитель меньше знаменателя
    if (first_is_less(numerator, denominator))
        return {vector<Digit>{0}, numerator};

    // Нормализующий множитель
    Digit scale = base / (denominator.back() + 1);
    assert(scale <= base / 2); // Старшая цифра denominator не может быть 0, т.е. минимум 1

    // Нормализованный числитель
    vector<Digit> norm_numerator = (scale == 1)
        ? numerator
        : mul_magnitudes(vector<Digit>{scale}, numerator);

    // Нормализованный знаменатель
    vector<Digit> norm_denominator = (scale == 1)
        ? denominator
        : mul_magnitudes(vector<Digit>{scale}, denominator);

    // Неполное частное
    vector<Digit> quotient;
    quotient.reserve(norm_numerator.size() - norm_denominator.size() + 1);

    // Текущий кусок числителя (и одновременно остаток)
    vector<Digit> chunk;
    quotient.reserve(norm_denominator.size() + 1);

    // Копируем цифры числителя, начиная с конца (со старших разрядов)
    for (size_t i = norm_numerator.size() - 1; i != size_t(-1); --i)
    {
        // Копируем очередную цифру числителя в начало куска (так как цифры хранятся в обратном порядке)
        chunk.insert(chunk.begin(), norm_numerator[i]); // chunk = chunk * base + norm_numerator[i]

        // Убираем ведущие нули (когда chunk поделился без остатка и стал равен 0, а потом добавили 0)
        while (chunk.size() > 1 && chunk.back() == 0)
            chunk.pop_back();

        // Делим кусок на знаменатель и получаем очередную цифру результата
        Digit digit = div_chunk(chunk, norm_denominator);

        // Добавляем цифру к результату
        quotient.push_back(digit);

        // кусок -= цифра * знаменатель ⇔ кусок %= знаменатель
        chunk = sub_magnitudes(chunk, mul_magnitudes(vector<Digit>{digit}, norm_denominator));
    }

    // Мы добавляли цифры в конец вектора, а не в начало, поэтому реверсируем
    reverse(quotient.begin(), quotient.end());

    // Убираем ведущие нули
    while (quotient.size() > 1 && quotient.back() == 0)
        quotient.pop_back();

    // Денормализуем остаток
    if (scale != 1)
    {
        pair<vector<Digit>, vector<Digit>> denorm_chunk = div_by_digit(chunk, scale);
        assert(denorm_chunk.second == vector<Digit>{0}); // Должно поделиться без остатка
        chunk = denorm_chunk.first;
    }

    return {quotient, chunk}; // В chunk находится остаток
}

// Генерирует случайное число из диапазона [min, max] (включительно)
static uint32_t generate_random(uint32_t min, uint32_t max)
{
    // Используем random_device только для генерации seed, так как он медленный
    static random_device device;
    static uint32_t random_seed = device();
    static mt19937 generator(random_seed);

    uniform_int_distribution<uint32_t> dist(min, max);
    return dist(generator);
}

BigInt BigInt::generate(size_t length)
{
    vector<BigInt::Digit> magnitude(length);

    // Старший разряд не должен быть 0
    magnitude[length - 1] = generate_random(1, BigInt::base - 1);

    // Остальные разряды могут быть 0
    for (size_t i = 0; i < length - 1; ++i)
        magnitude[i] = generate_random(0, BigInt::base - 1);

    BigInt ret;
    ret.positive_ = true;
    ret.magnitude_ = magnitude;

    return ret;
}

BigInt::BigInt()
{
    positive_ = true;
    magnitude_.push_back(0);
}

BigInt::BigInt(int32_t value)
{
    positive_ = (value >= 0);

    // Сперва преобразуем в int64_t, чтобы не было UB в std::abs(-2147483648)
    uint64_t val = (uint64_t)std::abs((int64_t)value);

    while (val != 0)
    {
        Digit mod = val % base;
        magnitude_.push_back(mod);
        val /= base;
    }

    if (!magnitude_.size()) // value == 0
        magnitude_.push_back(0);
}

BigInt::BigInt(uint32_t value)
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

// Обходит undefined behavior в std::abs()
static uint64_t fixed_abs(int64_t value)
{
    static_assert(is_same_v<int64_t, decltype(-9223372036854775807 - 1)>
        && is_same_v<uint64_t, decltype(9223372036854775808u)>);

    if (value == -9223372036854775807 - 1)
        return 9223372036854775808u;
    else
        return (uint64_t)std::abs(value);
}

BigInt::BigInt(int64_t value)
{
    positive_ = (value >= 0);

    uint64_t val = fixed_abs(value);

    while (val != 0)
    {
        Digit mod = val % base;
        magnitude_.push_back(mod);
        val /= base;
    }

    if (!magnitude_.size()) // value == 0
        magnitude_.push_back(0);
}

BigInt::BigInt(uint64_t value)
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

static uint32_t to_uint32_t(const string& chunk)
{
    unsigned long long val = stoull(chunk);
    assert(val <= base);
    return (uint32_t)val;
}

BigInt::BigInt(const string& str)
{
    if (str.empty())
    {
        // Инцициализируем нулём
        positive_ = true;
        magnitude_.push_back(0);
        return;
    }

    size_t first_digit_pos;

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

    // Проверяем, что после знака только цифры
    for (size_t i = first_digit_pos; i < str.length(); ++i)
    {
        if (!isdigit(str[i]))
        {
            // Инцициализируем нулём
            positive_ = true;
            magnitude_.push_back(0);
            return;
        }
    }

    // Пропускаем ведущие нули
    for (; first_digit_pos < str.length(); ++first_digit_pos)
    {
        if (str[first_digit_pos] != '0')
            break;
    }

    // Если после знака только нули или ничего нет
    if (first_digit_pos == str.length())
    {
        // Инцициализируем нулём
        positive_ = true;
        magnitude_.push_back(0);
        return;
    }

    size_t num_digits = str.length() - first_digit_pos;
    size_t num_chunks = num_digits / chunk_length;
    size_t rest_length = num_digits - num_chunks * chunk_length;
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

    magnitude_.reserve(num_chunks);

    // Извлекаем куски в обратном порядке и преобразуем в Digit (кроме первого куска)
    for (size_t i = 0; i < num_chunks - 1; ++i) // i - номер куска с конца
    {
        size_t chunk_start = str.size() - (i + 1) * chunk_length;
        string chunk = str.substr(chunk_start, chunk_length);
        magnitude_.push_back(to_uint32_t(chunk));
    }

    // Преобразуем в Digit первый кусок
    string first_chunk = str.substr(first_digit_pos, first_chunk_length);
    magnitude_.push_back(to_uint32_t(first_chunk));
}

bool BigInt::is_zero() const
{
    assert(magnitude_.size() > 0);

    // Проверяем, что нет ведущих нулей (вдруг число ещё не нормализовано и состоит из одних нулей)
    assert(magnitude_.size() == 1 || magnitude_.back() != 0);

    if (magnitude_.size() == 1 && magnitude_[0] == 0)
    {
        assert(positive_);
        return true;
    }

    return false;
}

void BigInt::set_positive(bool positive)
{
    if (!is_zero()) // Ноль всегда положительный
        positive_ = positive;
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
    for (size_t i = magnitude_.size() - 1; i != size_t(-1); --i)
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
        ret.magnitude_ = add_magnitudes(magnitude_, rhs.magnitude_);
    }
    else
    {
        if (first_is_less(magnitude_, rhs.magnitude_))
        {
            ret.positive_ = rhs.positive_;
            ret.magnitude_ = sub_magnitudes(rhs.magnitude_, magnitude_);
        }
        else
        {
            ret.positive_ = positive_;
            ret.magnitude_ = sub_magnitudes(magnitude_, rhs.magnitude_);
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
        ret.magnitude_ = add_magnitudes(magnitude_, rhs.magnitude_);
    }
    else
    {
        if (first_is_less(magnitude_, rhs.magnitude_))
        {
            ret.positive_ = !rhs.positive_;
            ret.magnitude_ = sub_magnitudes(rhs.magnitude_, magnitude_);
        }
        else
        {
            ret.positive_ = positive_;
            ret.magnitude_ = sub_magnitudes(magnitude_, rhs.magnitude_);
        }
    }

    return ret;
}

BigInt BigInt::operator*(const BigInt& rhs) const
{
    BigInt ret;
    ret.magnitude_ = mul_magnitudes(magnitude_, rhs.magnitude_);
    ret.set_positive(positive_ == rhs.positive_);
    return ret;
}

BigInt BigInt::operator/(const BigInt& rhs) const
{
    pair<vector<Digit>, vector<Digit>> dm = div_mod_magnitudes(magnitude_, rhs.magnitude_);

    BigInt ret;
    ret.magnitude_ = dm.first; // Неполное частное

    // https://en.cppreference.com/w/cpp/language/operator_arithmetic#Built-in_multiplicative_operators
    // (a/b)*b + a%b == a
    // 7/3 = {2, 1}    | 2*3 + 1 == 7
    // -7/3 = {-2, -1} | -2*3 + -1 == -7
    // 7/-3 = {-2, 1}  | -2*-3 + 1 == 7
    // -7/-3 = {2, -1} | 2*-3 + -1 == -7

    if (positive_ != rhs.positive_) // Если знаки числителя и знаменателя разные
        ret.set_positive(false); // Тогда неполное частное отрицательное

    return ret;
}

BigInt BigInt::operator%(const BigInt& rhs) const
{
    pair<vector<Digit>, vector<Digit>> dm = div_mod_magnitudes(magnitude_, rhs.magnitude_);

    BigInt ret;
    ret.magnitude_ = dm.second; // Остаток

    // Знак остатка совпадает со знаком числителя
    ret.set_positive(positive_);

    return ret;
}

BigInt BigInt::operator-() const
{
    BigInt ret = *this;
    ret.set_positive(!positive_);
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

string BigInt::to_string() const
{
    assert(magnitude_.size() > 0);

    string ret;

    if (!positive_)
    {
        ret.reserve(magnitude_.size() * chunk_length + 1);
        ret = "-";
    }
    else
    {
        ret.reserve(magnitude_.size() * chunk_length);
    }

    // Первый кусок результата без ведущих нулей
    ret += std::to_string(magnitude_.back());

    // Остальные куски результата с ведущими нулями
    for (size_t i = magnitude_.size() - 2; i != size_t(-1); --i)
        ret += format("{0:0>{1}}", magnitude_[i], chunk_length);

    return ret;
}

} // namespace dviglo
