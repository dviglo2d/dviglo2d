// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <cstdint> // uint64_t, ...
#include <string>
#include <vector>


namespace dviglo
{

class BigInt
{
public:

#if 1
    static constexpr uint32_t base = 1'000'000'000;

    /// Число десятичных цифр в каждой цифре Digit при преобразовании в строку и обратно
    static constexpr size_t chunk_length = 9;
#elif 1 // Для тестов
    static constexpr uint32_t base = 1'000;
    static constexpr size_t chunk_length = 3;
#else
    static constexpr uint32_t base = 1'0;
    static constexpr size_t chunk_length = 1;
#endif

    /// Отдельная цифра длинного числа
    using Digit = uint32_t;

    /// Генерирует случайное положительное число указанной длины. Никогда не генерирует 0
    static BigInt generate(size_t length);

private:
    /// Знак числа (ноль всегда положительный)
    bool positive_;

    /// Цифры числа в обратном порядке. Всегда содержит как минимум одну цифру
    std::vector<Digit> magnitude_;

public:
    /// Инициализирует нулём
    BigInt();

    BigInt(int32_t value);
    BigInt(uint32_t value);
    BigInt(int64_t value);
    BigInt(uint64_t value);
    BigInt(const std::string& str);

    bool is_zero() const;
    bool is_positive() const { return positive_; }
    bool is_negative() const { return !positive_; }
    void set_positive(bool positive);

    bool operator==(const BigInt& rhs) const { return positive_ == rhs.positive_ && magnitude_ == rhs.magnitude_; }
    std::strong_ordering operator<=>(const BigInt& rhs) const;

    BigInt operator+(const BigInt& rhs) const;
    BigInt operator-(const BigInt& rhs) const;

    BigInt operator*(const BigInt& rhs) const;

    /// Возвращает 0, если rhs ноль
    BigInt operator/(const BigInt& rhs) const;

    /// Возвращаеть 0, если rhs ноль
    BigInt operator%(const BigInt& rhs) const;

    BigInt operator-() const;

    BigInt& operator+=(const BigInt& rhs);
    BigInt& operator-=(const BigInt& rhs);
    BigInt& operator*=(const BigInt& rhs);
    BigInt& operator/=(const BigInt& rhs);
    BigInt& operator%=(const BigInt& rhs);

    /// Prefix increment operator
    BigInt& operator++() { this->operator+=(1); return *this; }

    /// Postfix increment operator
    BigInt operator++(int) { BigInt ret = *this; ++*this; return ret; }

    /// Prefix decrement operator
    BigInt& operator--() { this->operator-=(1); return *this; }

    /// Postfix decrement operator
    BigInt operator--(int) { BigInt ret = *this; --*this; return ret; }

    std::string to_string() const;
};

inline BigInt operator +(int32_t lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }
inline BigInt operator +(uint32_t lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }
inline BigInt operator +(int64_t lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }
inline BigInt operator +(uint64_t lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }

inline BigInt operator -(int32_t lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }
inline BigInt operator -(uint32_t lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }
inline BigInt operator -(int64_t lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }
inline BigInt operator -(uint64_t lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }

inline BigInt operator *(int32_t lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }
inline BigInt operator *(uint32_t lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }
inline BigInt operator *(int64_t lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }
inline BigInt operator *(uint64_t lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }

inline BigInt operator /(int32_t lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }
inline BigInt operator /(uint32_t lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }
inline BigInt operator /(int64_t lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }
inline BigInt operator/(uint64_t lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }

inline BigInt operator %(int32_t lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }
inline BigInt operator %(uint32_t lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }
inline BigInt operator %(int64_t lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }
inline BigInt operator %(uint64_t lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }

inline BigInt operator ""_bi(const char* str, std::size_t) { return BigInt(str); }

inline BigInt abs(const BigInt& value) { return value.is_negative() ? -value : value; }

} // namespace dviglo
