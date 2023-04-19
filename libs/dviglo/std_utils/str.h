// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

// Файл называется str.h, а не string.h, так как в стандартной библиотеке
// уже есть файл string.h

#pragma once

#include "../common/config.h"
#include "../common/primitive_types.h"

#include <algorithm>
#include <string>


namespace dviglo
{

constexpr bool contains(std::string_view ascii_str, char ascii_c) noexcept
{
    return ascii_str.find(ascii_c) != std::string::npos;
}

constexpr bool contains(std::string_view utf8_str, std::string_view utf8_substr) noexcept
{
    return utf8_str.find(utf8_substr) != std::string::npos;
}

constexpr std::string replace_all(std::string_view utf8_str,
                                  std::string_view utf8_old_substr, std::string_view utf8_new_substr)
{
    std::string ret;

    std::size_t offset = 0;
    std::size_t pos = utf8_str.find(utf8_old_substr); // Позиция utf8_old_substr в исходной строке

    while (pos != std::string::npos)
    {
        ret.append(utf8_str, offset, pos - offset); // Копируем фрагмент до найденной подстроки
        ret += utf8_new_substr;
        offset = pos + utf8_old_substr.length(); // Смещение после найденной подстроки
        pos = utf8_str.find(utf8_old_substr, offset);
    }

    ret += utf8_str.substr(offset); // Копируем остаток строки

    return ret;
}

/// В отличие от std::tolower() может работать в compile time
constexpr char to_lower(char ascii_c)
{
    return (ascii_c >= 'A' && ascii_c <= 'Z') ? ascii_c + ('a' - 'A') : ascii_c;
}

/// В отличие от std::toupper() может работать в compile time
constexpr char to_upper(char ascii_c)
{
    return (ascii_c >= 'a' && ascii_c <= 'z') ? ascii_c - ('a' - 'A') : ascii_c;
}

constexpr std::string replace_all(std::string_view ascii_str,
                                  char ascii_old_c, char ascii_new_c,
                                  bool case_sensitive = true)
{
    std::string ret(ascii_str);

    if (case_sensitive)
    {
        std::replace(ret.begin(), ret.end(), ascii_old_c, ascii_new_c);
    }
    else
    {
        ascii_old_c = to_lower(ascii_old_c);

        for (size_t i = 0; i < ascii_str.length(); ++i)
        {
            if (to_lower(ret[i]) == ascii_old_c)
                ret[i] = ascii_new_c;
        }
    }

    return ret;
}

/// Извлекает очередной символ из UTF-8-строки (декодируя в UTF-32).
/// Использование:
/// size_t offset = 0;
/// while (offset < str.length()) { c32 code_point = next_code_point(str, offset); }
constexpr c32 next_code_point(std::string_view utf8_str, size_t& offset)
{
    // goto не разрешены в constexpr функциях, поэтому вместо перехода в конец функции используем макрос 
    #define NEXT_CODE_POINT_ERROR \
        { \
            /* При ошибке прерываем декодирование строки */ \
            offset = utf8_str.length(); \
            /* Необходимо что-то вернуть */ \
            return '?'; \
        }

    if (offset >= utf8_str.length())
        return 0;

    // Получаем байт из строки и после этого инкрементируем смещение
    u8 byte1 = utf8_str[offset++];

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
        if (offset == utf8_str.length()) // Проверяем, что второй байт вообще есть (offset уже указывает на очередной байт)
            NEXT_CODE_POINT_ERROR

        // Получаем второй байт последовательности
        u8 byte2 = utf8_str[offset++];

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
        if (offset + 1 >= utf8_str.length())
            NEXT_CODE_POINT_ERROR

        u8 byte2 = utf8_str[offset++];
        u8 byte3 = utf8_str[offset++];

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
        if (offset + 2 >= utf8_str.length())
            NEXT_CODE_POINT_ERROR

        u8 byte2 = utf8_str[offset++];
        u8 byte3 = utf8_str[offset++];
        u8 byte4 = utf8_str[offset++];

        if ((byte2 & 0b11000000u) != 0b10000000u || (byte3 & 0b11000000u) != 0b10000000u || (byte4 & 0b11000000u) != 0b10000000u)
            NEXT_CODE_POINT_ERROR

        return (byte4 & 0b00111111u) | ((byte3 & 0b00111111u) << 6u) | ((byte2 & 0b00111111u) << 12u) | ((byte1 & 0b00000111u) << 18u);
    }

    NEXT_CODE_POINT_ERROR
}

} // namespace dviglo
