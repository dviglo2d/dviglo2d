// Copyright (c) the Dviglo project
// License: MIT

#include "file.hpp"

#include "log.hpp"

#include <array>
#include <fstream>

using namespace std;


namespace dviglo
{

vector<byte> read_all_data(const fs::path& path)
{
    vector<byte> ret;

    // Открываем файл сразу в конце, чтобы получить его размер
    ifstream stream(path, ios::binary | ios::ate);
    if (!stream)
    {
        DV_LOG->writef_error("{} | !stream | {}", DV_FUNC_SIG, path);
        return ret;
    }

    // Определяем размер файла и выделяем память
    ret.resize(stream.tellg());

    // Возвращаемся в начало файла и читаем содержимое
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(ret.data()), ret.size());

    return ret;
}

StrUtf8 read_all_text(const fs::path& path)
{
    StrUtf8 ret;

    // Открываем файл сразу в конце, чтобы получить его размер
    ifstream stream(path, ios::binary | ios::ate);
    if (!stream)
    {
        DV_LOG->writef_error("{} | !stream | {}", DV_FUNC_SIG, path);
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
        DV_LOG->writef_error("{} | !stream | {}", DV_FUNC_SIG, path);
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
