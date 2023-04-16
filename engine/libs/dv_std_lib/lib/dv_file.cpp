// Copyright (c) the Dviglo project
// License: MIT

#include "dv_file.hpp"

#include <fstream>

using namespace std;


namespace dviglo
{

StrUtf8 read_all_text(const fs::path& path)
{
    StrUtf8 ret;

    // Открываем файл сразу в конце, чтобы получить его размер
    ifstream stream(path, ios::binary | ios::ate);
    if (!stream)
    {
        Log::writef_error("{} | !stream | {}", DV_FUNC_SIG, path);
        return ret;
    }

    // Определяем размер файла и выделяем память
    ret.resize(stream.tellg());

    // Возвращаемся в начало файла и читаем содержимое
    stream.seekg(0);
    stream.read(ret.data(), ret.size());

    return ret;
}

vector<StrUtf8> read_all_lines(const fs::path& path, bool ignore_empty)
{
    vector<StrUtf8> ret;
    StrUtf8 line;

    ifstream stream(path);
    if (!stream)
    {
        Log::writef_error("{} | !stream | {}", DV_FUNC_SIG, path);
        return ret;
    }

    while (getline(stream, line))
    {
        if (!(line.empty() && ignore_empty))
            ret.emplace_back(std::move(line));
    }

    return ret;
}

} // namespace dviglo
