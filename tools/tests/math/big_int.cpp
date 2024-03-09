// Copyright (c) the Dviglo project
// License: MIT

#include "../force_assert.hpp"

#include <dviglo/math/big_int.hpp>

using namespace dviglo;


void test_math_big_int()
{
    // BigInt()
    assert(BigInt().to_string() == "0");

    // BigInt(i32 value)
    assert(BigInt(0).to_string() == "0");
    assert(BigInt(-0).to_string() == "0");
    assert(BigInt(-1).to_string() == "-1");
    assert(BigInt(322).to_string() == "322");

    // Типы i8, u8, i16, u16 всегда используют конструктор BigInt(i32 value)
    assert(BigInt((i8)-12).to_string() == "-12");
    assert(BigInt((u8)-12).to_string() == "244");
    assert(BigInt((i16)-12).to_string() == "-12");
    assert(BigInt((u16)-12).to_string() == "65524");

    assert(BigInt(0x7FFFFFFF).to_string() == "2147483647"); // BigInt(i32 value)
    assert(BigInt(-0x7FFFFFFF).to_string() == "-2147483647"); // BigInt(i32 value)
    assert(BigInt(0x80000000).to_string() == "2147483648"); // BigInt(u32 value)
    assert(BigInt(-0x80000000).to_string() == "2147483648"); // BigInt(u32 value)
    assert(BigInt(-0x7FFFFFFF - 1).to_string() == "-2147483648"); // BigInt(i32 value)
    assert(BigInt((i32)-0x80000000).to_string() == "-2147483648"); // BigInt(i32 value)
    assert(BigInt((i32)0x80000000).to_string() == "-2147483648"); // BigInt(i32 value)
    assert(BigInt(0xFFFFFFFF).to_string() == "4294967295"); // BigInt(u32 value)

    assert(BigInt(0x7FFFFFFFFFFFFFFF).to_string() == "9223372036854775807"); // BigInt(i64 value)
    assert(BigInt(-0x7FFFFFFFFFFFFFFF).to_string() == "-9223372036854775807"); // BigInt(i64 value)
    assert(BigInt(0x8000000000000000).to_string() == "9223372036854775808"); // BigInt(u64 value)
    assert(BigInt(-0x8000000000000000).to_string() == "9223372036854775808"); // BigInt(u64 value)
    assert(BigInt(-0x7FFFFFFFFFFFFFFF - 1).to_string() == "-9223372036854775808"); // BigInt(i64 value)
    assert(BigInt((i64)-0x8000000000000000).to_string() == "-9223372036854775808"); // BigInt(i64 value)
    assert(BigInt((i64)0x8000000000000000).to_string() == "-9223372036854775808"); // BigInt(i64 value)
    assert(BigInt(0xFFFFFFFFFFFFFFFF).to_string() == "18446744073709551615"); // BigInt(u64 value)

    // BigInt(const StrUtf8& str)
    assert(BigInt("0").to_string() == "0");
    assert(BigInt("00000").to_string() == "0");
    assert(BigInt("-0").to_string() == "0");
    assert(BigInt("+00000").to_string() == "0");
    assert(BigInt("3").to_string() == "3");
    assert(BigInt("-7").to_string() == "-7");
    assert(BigInt("123456789").to_string() == "123456789");
    assert(BigInt("-123456789").to_string() == "-123456789");
    assert(BigInt("123000789").to_string() == "123000789");
    assert(BigInt("-1230000").to_string() == "-1230000");
    assert(BigInt("0001230000").to_string() == "1230000");
    assert(BigInt("-0001230000").to_string() == "-1230000");
    
    {
        BigInt bi("-99999999999999999999999999999999999999999999999999999999999999999999"
                  "999999999999999999999999999999999999999999999999999999999999999999999"
                  "999999999999999999999999999999999999999999999999999999999999999999999");
        
        assert(bi.to_string() ==
               "-99999999999999999999999999999999999999999999999999999999999999999999"
               "999999999999999999999999999999999999999999999999999999999999999999999"
               "999999999999999999999999999999999999999999999999999999999999999999999");
    }

    // Сравнение
    assert(BigInt("+0") == BigInt("-0"));
    assert(BigInt("10") != BigInt("100"));
    assert(BigInt("10") != BigInt("-10"));
    assert(BigInt("10") < BigInt("100"));
    assert(BigInt("10") <= BigInt("100"));
    assert(BigInt("10") > BigInt("-100"));
    assert(BigInt("-10") > BigInt("-100"));

    // Сумма чисел с одинаковыми знаками
    assert((BigInt("0") + BigInt("0")).to_string() == "0");
    assert((BigInt("000") + BigInt("000")).to_string() == "0");
    assert((BigInt("1") + BigInt("2")).to_string() == "3");
    assert((BigInt("1000") + BigInt("200")).to_string() == "1200");
    assert((BigInt("-1000") + BigInt("-234")).to_string() == "-1234");
    assert((BigInt("-1000") - BigInt("234")).to_string() == "-1234");
    assert((BigInt("-1000") - BigInt("0")).to_string() == "-1000");
    assert((BigInt("9999999999999999999999") + BigInt("9999999999999999999999")).to_string() == "19999999999999999999998");
    assert((BigInt("9999999999999999999999") + BigInt("1")).to_string() == "10000000000000000000000");
    assert((BigInt(1) + BigInt(0xFFFFFFFFFFFFFFFF)).to_string() == "18446744073709551616");
    assert((BigInt(0xFFFFFFFFFFFFFFFF) + BigInt(0xFFFFFFFFFFFFFFFF)).to_string() == "36893488147419103230");
    assert((BigInt("999999999") + 1).to_string() == "1000000000");

    // Сумма чисел с разными знаками
    assert((BigInt("000") - BigInt("000")).to_string() == "0");
    assert((BigInt("1000") - BigInt("1000")).to_string() == "0");
    assert((BigInt("1000") - BigInt("234")).to_string() == "766");
    assert((BigInt("234") - BigInt("1000")).to_string() == "-766");
    assert((BigInt("1000") - BigInt("0")).to_string() == "1000");
    assert((BigInt("0") - BigInt("034005")).to_string() == "-34005");
    assert((BigInt("10000000000000000000000") - BigInt("1")).to_string() == "9999999999999999999999");
    assert((BigInt("-10000000000000000000000") + BigInt("1")).to_string() == "-9999999999999999999999");
    assert((BigInt(1) - BigInt(0xFFFFFFFFFFFFFFFF)).to_string() == "-18446744073709551614");
    assert((BigInt("1000000000") - 1).to_string() == "999999999");

    // Умножение
    assert((BigInt("0") * BigInt("0")).to_string() == "0");
    assert((BigInt("1") * BigInt("1")).to_string() == "1");
    assert((BigInt("1") * BigInt("9999999999999999999999")).to_string() == "9999999999999999999999");
    assert((BigInt("0") * BigInt("9999999999999999999999")).to_string() == "0");
    assert((BigInt("10") * BigInt("2")).to_string() == "20");
    assert((BigInt("-99999") * BigInt("99999")).to_string() == "-9999800001");
    
    {
        BigInt bi1("-99999999999999999999999999999999999999999999999999999999999999999999");
        BigInt bi2("99999999999999999999999999999999999999999999999999999999999999999999");
        StrUtf8 str = (bi1 * bi2).to_string();
        
        assert(str == "-99999999999999999999999999999999999999999999999999999999999999999998"
                      "00000000000000000000000000000000000000000000000000000000000000000001");
    }

    // Деление
    assert((BigInt("0") / BigInt("-0")).to_string() == "0");
    assert((BigInt("0") % BigInt("0")).to_string() == "0");
    assert((BigInt("999999") / 1234).to_string() == "810");
    assert((BigInt("999999") % 1234).to_string() == "459");
    assert(1234 / BigInt("999999") == 0);
    assert(1234 % BigInt("999999") == 1234);

    {
        // https://en.cppreference.com/w/cpp/language/operator_arithmetic
        // (a/b)*b + a%b == a
        BigInt a("9999999843");
        BigInt b("99999998");
        assert(a/b*b + a%b == a);

        a = {"-9999999843"};
        b = {"99999998"};
        assert(a/b*b + a%b == a);

        a = {"9999999843"};
        b = {"-99999998"};
        assert(a/b*b + a%b == a);

        a = {"-9999999843"};
        b = {"-99999998"};
        assert(a/b*b + a%b == a);
    }

#if false // Текущая имплементация деления слишком медленная
    {
        BigInt num("-99999999999999999999999999999999999999999999999999999999999999999998"
                   "00000000000000000000000000000000000000000000000000000000000000000001");
            
        BigInt denom("-99999999999999999999999999999999999999999999999999999999999999999999");
        StrUtf8 str = (num / denom).to_string();
        assert(str == "99999999999999999999999999999999999999999999999999999999999999999999");
        str = (num % denom).to_string();
        assert(str == "0");
    }
#else
    {
        BigInt num("-999998"
                   "000001");
            
        BigInt denom("-999999");
        StrUtf8 str = (num / denom).to_string();
        assert(str == "999999");
        str = (num % denom).to_string();
        assert(str == "0");
    }
#endif

    // Дополнительные операторы
    {
        BigInt bi = 1;
        assert((bi++).to_string() == "1");
        assert(bi.to_string() == "2");
        assert((++bi).to_string() == "3");
        assert(bi.to_string() == "3");
        assert((bi--).to_string() == "3");
        assert(bi.to_string() == "2");
        assert((--bi).to_string() == "1");
        assert(bi.to_string() == "1");
        assert((bi += 10).to_string() == "11");
        assert((bi -= 2).to_string() == "9");
        assert((bi *= 2).to_string() == "18");
    }
}
