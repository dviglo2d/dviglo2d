// Copyright (c) the Dviglo project
// License: MIT

#include <dv_force_assert.hpp>

#include <dv_string.hpp>

using namespace dviglo;
using namespace std;


void test_string()
{
    {
        const StrAscii str("hello");
        assert(contains(str, 'e'));
    }

    {
        const StrUtf8 str("привет🍌");
        assert(contains(str, "🍌"));
        assert(contains(str, string("иве")));
    }

    {
        static_assert(to_lower('G') == 'g');
        static_assert(to_upper('d') == 'D');
    }

    {
        const StrUtf8 str("привет привет");
        assert(replace_all(str, "иве", "ави") == "правит правит");
    }

    {
        const StrAscii str = "heLlo";
        assert(replace_all(str, 'L', '2') == "he2lo");
        assert(replace_all(str, 'l', '2', false) == "he22o");
    }

    {
        StrAscii str = "heLlo\r\n";
        trim_end_chars(str, "\r\n");
        assert(str == "heLlo");
        trim_end_chars(str, "hLo");
        assert(str == "heLl");
        trim_end_chars(str, "Lleh");
        assert(str.empty());
    }

    {
        const StrUtf8 str = "привет🍌";
        size_t offset = 0;
        // https://en.cppreference.com/w/cpp/language/character_literal
        assert(next_code_point(str, offset) == U'п');
        assert(next_code_point(str, offset) == U'р');
        assert(next_code_point(str, offset) == U'и');
        assert(next_code_point(str, offset) == U'в');
        assert(next_code_point(str, offset) == U'е');
        assert(next_code_point(str, offset) == U'т');
        assert(next_code_point(str, offset) == U'🍌');
        assert(offset == str.length());
        assert(next_code_point(str, offset) == '?');
        assert(offset == str.length());
    }

    {
        // UTF-8 строка с повреждённой последовательностью
        const StrUtf8 str = "пр\xBFвет";
        size_t offset = 0;
        assert(next_code_point(str, offset) == U'п');
        assert(next_code_point(str, offset) == U'р');
        assert(next_code_point(str, offset) == '?'); // Первый байт не может начинаться с 0b10
        assert(offset == str.length());
    }

    {
        // UTF-8 строка с повреждённой последовательностью
        const StrUtf8 str = "пр\xDF\xFFвет";
        size_t offset = 0;
        assert(next_code_point(str, offset) == U'п');
        assert(next_code_point(str, offset) == U'р');
        assert(next_code_point(str, offset) == '?'); // Второй байт должен начинаться с 0b10
        assert(offset == str.length());
    }

    {
        assert(to_utf8(U'7') == "7");
        assert(to_utf8(U'п') == "п");
        assert(to_utf8(U'🍌') == "🍌");
    }

#ifdef DV_WINDOWS
    {
        const StrUtf8 str = "🍏привет🍌";
        const wstring wstr = to_wstring(str);
        assert(wstr[0] == L'\xd83c');
        assert(wstr[1] == L'\xdf4f');
        assert(wstr[2] == L'п');
        assert(wstr[3] == L'р');
        assert(wstr[4] == L'и');
        assert(wstr[5] == L'в');
        assert(wstr[6] == L'е');
        assert(wstr[7] == L'т');
        assert(wstr[8] == L'\xd83c');
        assert(wstr[9] == L'\xdf4c');
        assert(from_wstring(wstr) == "🍏привет🍌");
    }
#endif
}
