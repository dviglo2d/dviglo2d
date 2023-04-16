// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../common/primitive_types.hpp"

#include <algorithm>
#include <string>
#include <vector>


namespace dvt
{

using StrUtf8 = std::string;
using StrAscii = std::string;
using StrViewUtf8 = std::string_view;
using StrViewAscii = std::string_view;

} // namespace dvt

namespace dviglo
{

constexpr bool contains(StrViewAscii str, char ascii_c) noexcept
{
    return str.find(ascii_c) != StrViewAscii::npos;
}

// Используется также для поиска UTF-8-символов
constexpr bool contains(StrViewUtf8 str, StrViewUtf8 substr) noexcept
{
    return str.find(substr) != StrViewUtf8::npos;
}

// Используется также для замены UTF-8-символов
constexpr StrUtf8 replace_all(StrViewUtf8 str, StrViewUtf8 old_substr, StrViewUtf8 new_substr)
{
    StrUtf8 ret;

    size_t offset = 0;
    size_t pos = str.find(old_substr); // Позиция old_substr в исходной строке

    while (pos != StrViewUtf8::npos)
    {
        ret.append(str, offset, pos - offset); // Копируем фрагмент до найденной подстроки
        ret += new_substr;
        offset = pos + old_substr.length(); // Смещение после найденной подстроки
        pos = str.find(old_substr, offset);
    }

    ret += str.substr(offset); // Копируем остаток строки

    return ret;
}

// В отличие от std::tolower() может работать в compile time
constexpr char to_lower(char ascii_c)
{
    return (ascii_c >= 'A' && ascii_c <= 'Z') ? ascii_c + ('a' - 'A') : ascii_c;
}

// В отличие от std::toupper() может работать в compile time
constexpr char to_upper(char ascii_c)
{
    return (ascii_c >= 'a' && ascii_c <= 'z') ? ascii_c - ('a' - 'A') : ascii_c;
}

// Версия std::stoull(), которая не вызывает исключений
inline u64 to_u64(const StrUtf8& str)
{
    try
    {
        return std::stoull(str);
    }
    catch (...)
    {
        return 0;
    }
}

constexpr StrAscii replace_all(StrViewAscii str,
                               char old_ascii_c, char new_ascii_c,
                               bool case_sensitive = true)
{
    StrAscii ret(str);

    if (case_sensitive)
    {
        std::replace(ret.begin(), ret.end(), old_ascii_c, new_ascii_c);
    }
    else
    {
        old_ascii_c = to_lower(old_ascii_c);

        for (size_t i = 0; i < str.length(); ++i)
        {
            if (to_lower(ret[i]) == old_ascii_c)
                ret[i] = new_ascii_c;
        }
    }

    return ret;
}

// Укорачивает строку, если в конце неё находятся любые символы из набора
constexpr void trim_end_chars(StrAscii& str, StrViewAscii chars = " ")
{
    size_t new_length = str.length();

    while (new_length > 0)
    {
        char c = str.c_str()[new_length - 1];

        if (!contains(chars, c))
            break;

        --new_length;
    }

    if (new_length < str.length())
        str.resize(new_length);
}

constexpr StrUtf8 join(const std::vector<StrUtf8>& values, const StrUtf8& separator)
{
    StrUtf8 ret;

    for (const StrUtf8& value : values)
    {
        if (!ret.empty())
            ret += separator;

        ret += value;
    }

    return ret;
}

// Извлекает очередной символ из UTF-8-строки (декодируя в UTF-32).
// Использование:
// size_t offset = 0;
// while (offset < str.length()) { c32 code_point = next_code_point(str, offset); }
constexpr c32 next_code_point(StrViewUtf8 str, size_t& offset)
{
    // goto не разрешены в constexpr функциях, поэтому вместо перехода в конец функции используем макрос
    #define NEXT_CODE_POINT_ERROR \
        { \
            /* При ошибке прерываем декодирование строки */ \
            offset = str.length(); \
            /* Необходимо что-то вернуть */ \
            return '?'; \
        }

    if (offset >= str.length())
        return 0;

    // Получаем байт из строки и после этого инкрементируем смещение
    u8 byte1 = str[offset++];

    // Если первый бит первого байта - ноль, значит последовательность состоит из единственного байта.
    // 0xxxxxxx (x - биты, в которой хранится кодовая позиция)
    if (byte1 <= 0b01111111u)
        return byte1;

    // Если мы дошли досюда, значит byte1 > 01111111, то есть byte1 >= 01111111 + 1, то есть byte1 >= 10000000

    // Первый байт не может начинаться с 10. Или offset указывает не на первый байт последовательности, или ошибка в буфере
    if (byte1 <= 0b10111111u) // Выше мы уже проверили, что byte1 >= 10000000
        NEXT_CODE_POINT_ERROR

    // Если первый байт начинается с 110, то последовательность имеет длину 2 байта.
    // 110xxxxx 10xxxxxx (x - биты, в которой хранится кодовая позиция)
    if (byte1 <= 0b11011111u) // Выше мы уже проверили, что byte1 >= 11000000
    {
        if (offset == str.length()) // Проверяем, что второй байт вообще есть (offset уже указывает на очередной байт)
            NEXT_CODE_POINT_ERROR

        // Получаем второй байт последовательности
        u8 byte2 = str[offset++];

        // Проверяем, что второй байт начинается с 10
        if ((byte2 & 0b11000000u) != 0b10000000u)
            NEXT_CODE_POINT_ERROR

        // Выкидываем из байтов служебную информацию.
        // 0b00111111u имеет тип c32, поэтому тут всё автоматически преобразуется к c32
        return (byte2 & 0b00111111u) | ((byte1 & 0b00011111u) << 6u);
    }

    // Если первый байт начинается с 1110, то последовательность имеет длину 3 байта.
    // 1110xxxx 10xxxxxx 10xxxxxx
    if (byte1 <= 0b11101111u) // Выше мы уже проверили, что byte1 >= 11100000
    {
        if (offset + 1 >= str.length())
            NEXT_CODE_POINT_ERROR

        u8 byte2 = str[offset++];
        u8 byte3 = str[offset++];

        // Проверяем, что второй и третий байты начинаются с 10
        if ((byte2 & 0b11000000u) != 0b10000000u || (byte3 & 0b11000000u) != 0b10000000u)
            NEXT_CODE_POINT_ERROR

        // Выкидываем из байтов служебную информацию
        return (byte3 & 0b00111111u) | ((byte2 & 0b00111111u) << 6u) | ((byte1 & 0b00001111u) << 12u);
    }

    // Если первый байт начинается с 11110, то последовательность имеет длину 4 байта.
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if (byte1 <= 0b11110111u) // Выше мы уже проверили, что byte1 >= 11110000
    {
        if (offset + 2 >= str.length())
            NEXT_CODE_POINT_ERROR

        u8 byte2 = str[offset++];
        u8 byte3 = str[offset++];
        u8 byte4 = str[offset++];

        if ((byte2 & 0b11000000u) != 0b10000000u || (byte3 & 0b11000000u) != 0b10000000u || (byte4 & 0b11000000u) != 0b10000000u)
            NEXT_CODE_POINT_ERROR

        return (byte4 & 0b00111111u) | ((byte3 & 0b00111111u) << 6u) | ((byte2 & 0b00111111u) << 12u) | ((byte1 & 0b00000111u) << 18u);
    }

    NEXT_CODE_POINT_ERROR
}

constexpr std::vector<c32> to_utf32(StrViewUtf8 str)
{
    std::vector<c32> ret;

    size_t offset = 0;
    while (offset < str.length())
    {
        c32 code_point = next_code_point(str, offset);
        ret.push_back(code_point);
    }

    return ret;
}

// Кодирует кодовую позицию в последовательность кодовых квантов UTF-8.
// https://ru.wikipedia.org/wiki/UTF-8
constexpr StrUtf8 to_utf8(c32 utf32_c)
{
    StrUtf8 ret;

    if (utf32_c <= 0b01111111u) // 7-битный ASCII просто возвращаем
    {
        ret += static_cast<char>(utf32_c);
    }
    else if (utf32_c <= 0x7ffu)
    {
        ret.reserve(2);
        ret += static_cast<char>(0b110'00000u | ((utf32_c >> 6u) & 0b000'11111u));
        ret += static_cast<char>(0b10'000000u | (utf32_c         & 0b00'111111u));
    }
    else if (utf32_c <= 0xffffu)
    {
        ret.reserve(3);
        ret += static_cast<char>(0b1110'0000u | ((utf32_c >> 12u) & 0b0000'1111u));
        ret += static_cast<char>(0b10'000000u | ((utf32_c >> 6u)  & 0b00'111111u));
        ret += static_cast<char>(0b10'000000u | (utf32_c          & 0b00'111111u));
    }
    else if (utf32_c <= 0x10ffffu)
    {
        ret.reserve(4);
        ret += static_cast<char>(0b11110'000u | ((utf32_c >> 18u) & 0b00000'111u));
        ret += static_cast<char>(0b10'000000u | ((utf32_c >> 12u) & 0b00'111111u));
        ret += static_cast<char>(0b10'000000u | ((utf32_c >> 6u)  & 0b00'111111u));
        ret += static_cast<char>(0b10'000000u | (utf32_c          & 0b00'111111u));
    }
    else // Ошибка
    {
        ret += '?';
    }

    return ret;
}

#ifdef _WIN32

// Преобразует UTF-8 в UTF-16 little-endian для взаимодействия с Windows.
// Только в Windows размер wchar_t - 16 бит
constexpr std::wstring to_wstring(StrViewUtf8 str)
{
    std::wstring ret;

    size_t offset = 0;

    while (offset < str.length())
    {
        c32 code_point = next_code_point(str, offset);

        if (code_point < 0x10000) // Символы 0 - 0xffff кодируются как есть
        {
            ret += (wchar_t)code_point;
        }
        else
        {
            // Принцип кодирования описан тут: https://ru.wikipedia.org/wiki/UTF-16
            code_point -= 0x10000;
            ret += ((code_point >> 10) & 0b11'11111111) | 0xd800;
            ret += (code_point & 0b11'11111111) | 0xdc00;
        }
    }

    return ret;
}

// Преобразует UTF-16 little-endian в UTF-8 для взаимодействия с Windows.
// Только в Windows размер wchar_t - 16 бит
constexpr StrUtf8 from_wstring(std::wstring_view wstr)
{
    StrUtf8 ret;

    // Алгоритм декодирования: https://ru.wikipedia.org/wiki/UTF-16
    for (auto it = wstr.cbegin(); it != wstr.cend(); ++it)
    {
        wchar_t word1 = *it; // Получаем первый квант

        // Изучаем первый квант
        if (word1 < 0xd800 || word1 > 0xdfff) // Квант всего один
        {
            ret += to_utf8((c32)word1);
        }
        else if (word1 >= 0xdc00) // Ошибка
        {
            ret += '?';
            break;
        }
        else // Изучаем второй квант
        {
            if (++it == wstr.cend()) // Ошибка: нет второго кванта
            {
                ret += '?';
                break;
            }

            wchar_t word2 = *it; // Получаем второй квант

            if (word2 < 0xdc00 || word2 > 0xdfff) // Ошибка
            {
                ret += '?';
                break;
            }

            word1 = word1 & 0x3ff;
            word2 = word2 & 0x3ff;

            c32 code_point = ((word1 << 10) | word2) + 0x10000;
            ret += to_utf8(code_point);
        }
    }

    return ret;
}

#endif // def _WIN32

} // namespace dviglo
