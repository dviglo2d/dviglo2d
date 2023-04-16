// Copyright (c) the Dviglo project
// License: MIT

#include <dv_force_assert.hpp>

#include <chrono>
#include <dv_big_int.hpp>
#include <dv_locale.hpp>
#include <iostream>

using namespace dviglo;
using namespace std;


#ifdef _MSC_VER // VS
    #pragma warning(disable: 4146)
#endif

void run()
{
    // BigInt()
    assert(BigInt().to_string() == "0");

    // BigInt(int32_t value)
    assert(BigInt(0).to_string() == "0");
    assert(BigInt(-0).to_string() == "0");
    assert(BigInt(-1).to_string() == "-1");
    assert(BigInt(322).to_string() == "322");

    // Типы int8_t, uint8_t, int16_t, uint16_t всегда используют конструктор BigInt(int32_t value)
    assert(BigInt((int8_t)-12).to_string() == "-12");
    assert(BigInt((uint8_t)-12).to_string() == "244");
    assert(BigInt((int16_t)-12).to_string() == "-12");
    assert(BigInt((uint16_t)-12).to_string() == "65524");

    assert(BigInt(0x7FFFFFFF).to_string() == "2147483647"); // BigInt(int32_t value)
    assert(BigInt(-0x7FFFFFFF).to_string() == "-2147483647"); // BigInt(int32_t value)
    assert(BigInt(0x80000000).to_string() == "2147483648"); // BigInt(uint32_t value)
    assert(BigInt(-0x80000000).to_string() == "2147483648"); // BigInt(uint32_t value) // Warning ожидаем
    assert(BigInt(-0x7FFFFFFF - 1).to_string() == "-2147483648"); // BigInt(int32_t value)
    assert(BigInt((int32_t)-0x80000000).to_string() == "-2147483648"); // BigInt(int32_t value) // Warning ожидаем
    assert(BigInt((int32_t)0x80000000).to_string() == "-2147483648"); // BigInt(int32_t value)
    assert(BigInt(0xFFFFFFFF).to_string() == "4294967295"); // BigInt(uint32_t value)

    assert(BigInt(0x7FFFFFFFFFFFFFFF).to_string() == "9223372036854775807"); // BigInt(int64_t value)
    assert(BigInt(-0x7FFFFFFFFFFFFFFF).to_string() == "-9223372036854775807"); // BigInt(int64_t value)
    assert(BigInt(0x8000000000000000).to_string() == "9223372036854775808"); // BigInt(uint64_t value)
    assert(BigInt(-0x8000000000000000).to_string() == "9223372036854775808"); // BigInt(uint64_t value) // Warning ожидаем
    assert(BigInt(-0x7FFFFFFFFFFFFFFF - 1).to_string() == "-9223372036854775808"); // BigInt(int64_t value)
    assert(BigInt((int64_t)-0x8000000000000000).to_string() == "-9223372036854775808"); // BigInt(int64_t value) // Warning ожидаем
    assert(BigInt((int64_t)0x8000000000000000).to_string() == "-9223372036854775808"); // BigInt(int64_t value)
    assert(BigInt(0xFFFFFFFFFFFFFFFF).to_string() == "18446744073709551615"); // BigInt(uint64_t value)

    // BigInt(const string& str)
    assert(BigInt("").to_string() == "0");
    assert(BigInt("1abc").to_string() == "0");
    assert(BigInt("0").to_string() == "0");
    assert(BigInt("00000").to_string() == "0");
    assert(BigInt("-0").to_string() == "0");
    assert(BigInt("+00000").to_string() == "0");
    assert(BigInt("-").to_string() == "0");
    assert("-abc"_bi.to_string() == "0");
    assert("3"_bi.to_string() == "3");
    assert("-7"_bi.to_string() == "-7");
    assert("123456789"_bi.to_string() == "123456789");
    assert("-123456789"_bi.to_string() == "-123456789");
    assert("123000789"_bi.to_string() == "123000789");
    assert("-1230000"_bi.to_string() == "-1230000");
    assert("0001230000"_bi.to_string() == "1230000");
    assert("-0001230000"_bi.to_string() == "-1230000");

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
    assert("+0"_bi == "-0"_bi);
    assert("10"_bi != "100"_bi);
    assert("10"_bi != "-10"_bi);
    assert("10"_bi < "100"_bi);
    assert("10"_bi <= "100"_bi);
    assert("10"_bi > "-100"_bi);
    assert("-10"_bi > "-100"_bi);

    // Сумма чисел с одинаковыми знаками
    assert(("0"_bi + "0"_bi).to_string() == "0");
    assert(("000"_bi + "000"_bi).to_string() == "0");
    assert(("1"_bi + "2"_bi).to_string() == "3");
    assert(("1000"_bi + "200"_bi).to_string() == "1200");
    assert(("-1000"_bi + "-234"_bi).to_string() == "-1234");
    assert(("-1000"_bi - "234"_bi).to_string() == "-1234");
    assert(("-1000"_bi - "0"_bi).to_string() == "-1000");
    assert(("9999999999999999999999"_bi + "9999999999999999999999"_bi).to_string() == "19999999999999999999998");
    assert(("9999999999999999999999"_bi + "1"_bi).to_string() == "10000000000000000000000");
    assert((BigInt(1) + 0xFFFFFFFFFFFFFFFF).to_string() == "18446744073709551616");
    assert((BigInt(0xFFFFFFFFFFFFFFFF) + 0xFFFFFFFFFFFFFFFF).to_string() == "36893488147419103230");
    assert(("999999999"_bi + 1).to_string() == "1000000000");

    // Сумма чисел с разными знаками
    assert(("000"_bi - "000"_bi).to_string() == "0");
    assert(("1000"_bi - "1000"_bi).to_string() == "0");
    assert(("1000"_bi - "234"_bi).to_string() == "766");
    assert(("234"_bi - "1000"_bi).to_string() == "-766");
    assert(("1000"_bi - "0"_bi).to_string() == "1000");
    assert(("0"_bi - "034005"_bi).to_string() == "-34005");
    assert(("10000000000000000000000"_bi - "1"_bi).to_string() == "9999999999999999999999");
    assert(("-10000000000000000000000"_bi + "1"_bi).to_string() == "-9999999999999999999999");
    assert((BigInt(1) - 0xFFFFFFFFFFFFFFFF).to_string() == "-18446744073709551614");
    assert(("1000000000"_bi - 1).to_string() == "999999999");

    // Умножение
    assert(("0"_bi * "0"_bi).to_string() == "0");
    assert(("1"_bi * "1"_bi).to_string() == "1");
    assert(("1"_bi * "9999999999999999999999"_bi).to_string() == "9999999999999999999999");
    assert(("0"_bi * "9999999999999999999999"_bi).to_string() == "0");
    assert(("10"_bi * "2"_bi).to_string() == "20");
    assert(("-99999"_bi * "99999"_bi).to_string() == "-9999800001");
    assert(("-99999"_bi * 0).is_zero());

    {
        BigInt bi1("-99999999999999999999999999999999999999999999999999999999999999999999");
        BigInt bi2("99999999999999999999999999999999999999999999999999999999999999999999");
        string str = (bi1 * bi2).to_string();

        assert(str == "-99999999999999999999999999999999999999999999999999999999999999999998"
                      "00000000000000000000000000000000000000000000000000000000000000000001");
    }

    // Деление
    assert(("0"_bi / "-0"_bi).to_string() == "0");
    assert(("0"_bi % "0"_bi).to_string() == "0");
    assert((0 % "-99999"_bi).is_zero());
    assert((0 / "-99999"_bi).is_zero());
    assert(("-99999"_bi / 0 ).is_zero());
    assert(("-99999"_bi % 0).is_zero());
    assert(("999999"_bi / 1234).to_string() == "810");
    assert(("999999"_bi % 1234).to_string() == "459");
    assert(1234 / "999999"_bi == 0);
    assert(1234 % "999999"_bi == 1234);
    assert("15000000000"_bi / 5 == 3000000000);
    assert("15000000000"_bi % 5 == 0);

//#define DV_DIV_BENCHMARK 1

#if DV_DIV_BENCHMARK
    for (size_t i = 0; i < 500; ++i)
    {
#endif

    {
        BigInt num("231938472342346234324323000000000000000000000000000000002319384723423462343243229");
        BigInt denom("231938472342346234324323");
        assert((num / denom).to_string() == "1000000000000000000000000000000000000000000000000000000009");
        assert((num % denom).to_string() == "231938472342346234324322");
    }

    {
        BigInt num("-99999999999999999999999999999999999999999999999999999999999999999998"
                   "00000000000000000000000000000000000000000000000000000000000000000001");

        BigInt denom("-99999999999999999999999999999999999999999999999999999999999999999999");
        string str = (num / denom).to_string();
        assert(str == "99999999999999999999999999999999999999999999999999999999999999999999");
        str = (num % denom).to_string();
        assert(str == "0");
    }

    {
        BigInt num("-435802934583983490082374236423984934573467632463298429342384273947239"
                   "4092837842374203943249384238234349872398472742934729342634293423466324"
                   "2648901364732130846878735410663454303254567487340544878735487765435465"
                   "0743776536457436534765347653567346573457734736475348573456376547982347");

        BigInt denom("1574");
        string str = (num / denom).to_string();

        assert(str == "-27687607025666041301294424169249360519280027475431920542718187"
                      "671362097159071425503201672486558057397394360854337629826742964"
                      "639729501056697998031362599829000558281709443109468395532075819"
                      "914154215684730263954939248826966800288104532887774291396026539"
                      "6910085990199146363753483");

        str = (num % denom).to_string();
        assert(str == "-105");
    }

    {
        BigInt num("-349085734578253735348534758935835793475395734759352656347535634755774"
                   "0723580375234537583583576347563752502027356273639176269161951915925632"
                   "0014317353754375627953238756791061730147353378930226652053001376145017"
                   "0425734653617324023456200264301460307750443478653045610544743561010347");

        BigInt denom("-503485083745737260309431837463722349240239482347237423742734082340488"
                     "4981938283048785710573643765473563256295475639847513745638568435638456"
                     "0283451320657365783456387416275462738456832574567483538456348564382223"
                     "09374653294569378456392652346583573175463856345638456323");

        string str = (num / denom).to_string();
        assert(str == "69333878171958");

        str = (num % denom).to_string();

        assert(str == "-355277755598817268744652856648785435651554474282312186955910523919129"
                      "5638576798214686747968542159396972700770195868886908838686710009987281"
                      "9161566184986846734557785127378321974842925484023620272716472703539239"
                      "06043797282605802653416184941793268609238254235294619913");
    }

#if DV_DIV_BENCHMARK
    }
#endif

    {
        // https://en.cppreference.com/w/cpp/language/operator_arithmetic#Built-in_multiplicative_operators
        // (a/b)*b + a%b == a
        BigInt a("9999999843");
        BigInt b("99999998");
        assert(a/b*b + a%b == a);

        a = "-9999999843"_bi;
        b = "99999998"_bi;
        assert(a/b*b + a%b == a);

        a = "9999999843"_bi;
        b = "-99999998"_bi;
        assert(a/b*b + a%b == a);

        a = "-9999999843"_bi;
        b = "-99999998"_bi;
        assert(a/b*b + a%b == a);

        a = 0;
        b = "-99999998"_bi;
        assert(a/b*b + a%b == a);

        // Этот тест не должен быть успешным. Хотя деление на 0 и возвращает 0
        // без срабатывания исключения, но формула не работает
        //a = "-99999998"_bi;
        //b = 0;
        //assert(a/b*b + a%b == a);
    }

//#define DV_DIV_RANDOM 1

#if DV_DIV_RANDOM
    {
        // Для отладки багов в делении
        for (size_t i = 0; i < 10000; ++i)
        {
            BigInt a = BigInt::generate(4);
            BigInt b = BigInt::generate(3);
            cout << "Random a = " << a.to_string() << endl;
            cout << "Random b = " << b.to_string() << endl;
            assert(a/b*b + a%b == a);
        }
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

int main()
{
    assert(set_utf8_locale());

    auto begin_time = chrono::high_resolution_clock::now();

    run();

    auto end_time = chrono::high_resolution_clock::now();
    auto duration_ms = chrono::duration_cast<chrono::milliseconds>(end_time - begin_time).count();

    cout << "Все тесты пройдены успешно" << endl;
    cout << "Время: " << duration_ms << " мс" << endl;

    return 0;
}
