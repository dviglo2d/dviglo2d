// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/fs.hpp"
#include "../std_utils/string.hpp"


namespace dviglo
{

// Читает содержимое всего файла в вектор
std::vector<byte> read_all_data(const fs::path& path);

// Читает содержимое всего файла в строку. Файл должен быть в кодировке UTF-8 без BOM
StrUtf8 read_all_text(const fs::path& path);

// Считывает все строки из файла в вектор. Файл должен быть в кодировке UTF-8 без BOM
std::vector<StrUtf8> read_all_lines(const fs::path& path, bool ignore_empty = false);

} // namespace dviglo
