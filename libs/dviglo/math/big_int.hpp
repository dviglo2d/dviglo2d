// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.hpp"


namespace dviglo
{

class BigInt
{
public:
    /// В двоичной системе счисления 2 цифры: 0, 1.
    /// В десятичной системе счисления 10 цифр: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9.
    /// В шестнадцатеричной системе счисления 16 цифр: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F.
    /// В BigInt используется система счисления с основанием 1'000'000'000: 0, 1, 3, ..., 10^9 - 1,
    /// так как её легко преобразовывать в десятичную.
    /// Для хранения каждой цифры используется тип u64, так как он может хранить квадрат любой цифры
    using Digit = u64;

private:
    /// Знак числа (ноль всегда положительный)
    bool positive_;

    /// Цифры числа в обратном порядке. Всегда содержит как минимум одну цифру
    std::vector<Digit> magnitude_;

public:
    /// Инициализирует нулём
    BigInt();

    BigInt(i32 value);
    BigInt(u32 value);
    BigInt(i64 value);
    BigInt(u64 value);
    BigInt(const StrUtf8& str);

    bool is_positive() const { return positive_; }
    bool is_negative() const { return !positive_; }
    bool is_zero() const;

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

    StrUtf8 to_string() const;
};

inline BigInt operator+(i32 lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }
inline BigInt operator+(u32 lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }
inline BigInt operator+(i64 lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }
inline BigInt operator+(u64 lhs, const BigInt& rhs) { return BigInt(lhs) + rhs; }

inline BigInt operator-(i32 lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }
inline BigInt operator-(u32 lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }
inline BigInt operator-(i64 lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }
inline BigInt operator-(u64 lhs, const BigInt& rhs) { return BigInt(lhs) - rhs; }

inline BigInt operator*(i32 lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }
inline BigInt operator*(u32 lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }
inline BigInt operator*(i64 lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }
inline BigInt operator*(u64 lhs, const BigInt& rhs) { return BigInt(lhs) * rhs; }

inline BigInt operator/(i32 lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }
inline BigInt operator/(u32 lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }
inline BigInt operator/(i64 lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }
inline BigInt operator/(u64 lhs, const BigInt& rhs) { return BigInt(lhs) / rhs; }

inline BigInt operator%(i32 lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }
inline BigInt operator%(u32 lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }
inline BigInt operator%(i64 lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }
inline BigInt operator%(u64 lhs, const BigInt& rhs) { return BigInt(lhs) % rhs; }

inline BigInt abs(const BigInt& value) { return value.is_negative() ? -value : value; }

} // namespace dviglo
