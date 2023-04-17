// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "str.h"

using namespace std;


namespace dviglo
{

constexpr string replace_all(string_view str, string_view old_substr, string_view new_substr)
{
    string ret;

    size_t offset = 0;
    size_t pos = str.find(old_substr); // Позиция old_value в исходной строке

    while (pos != string::npos)
    {
        ret.append(str, offset, pos - offset); // Копируем фрагмент до найденной подстроки
        ret += new_substr;
        offset = pos + old_substr.length(); // Смещение после найденной подстроки
        pos = str.find(old_substr, offset);
    }

    ret += str.substr(offset); // Копируем остаток строки

    return ret;
}

} // namespace dviglo
