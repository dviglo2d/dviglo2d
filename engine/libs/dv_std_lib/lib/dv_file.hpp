// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_fs.hpp"
#include "dv_log.hpp"
#include "dv_string.hpp"


namespace dviglo
{

// Читает содержимое всего файла в вектор
template <typename T = byte>
requires std::same_as<T, byte> || std::same_as<T, char> || std::same_as<T, u32>
std::vector<T> read_all_data(const fs::path& path)
{
    std::vector<T> ret;

    // Открываем файл сразу в конце, чтобы получить его размер
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream)
    {
        Log::writef_error("{} | !stream | {}", DV_FUNC_SIG, path);
        return ret;
    }

    size_t file_size = static_cast<size_t>(stream.tellg()); // Размер файла в байтах

    if (!file_size)
        return ret; // Пустой файл

    if (file_size % sizeof(T) != 0)
    {
        Log::writef_error("{} | file_size % sizeof(T) != 0 | {}", DV_FUNC_SIG, path);
        return ret;
    }

    // Выделяем память
    ret.resize(file_size / sizeof(T));

    // Возвращаемся в начало файла и читаем содержимое
    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(ret.data()), file_size);

    return ret;
}

// Читает содержимое всего файла в строку. Файл должен быть в кодировке UTF-8 без BOM
StrUtf8 read_all_text(const fs::path& path);

// Считывает все строки из файла в вектор. Файл должен быть в кодировке UTF-8 без BOM
std::vector<StrUtf8> read_all_lines(const fs::path& path, bool ignore_empty = false);

} // namespace dviglo
