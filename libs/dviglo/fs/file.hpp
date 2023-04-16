// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/string.hpp"


namespace dviglo
{

/// Читает содержимое всего файла в строку. Файл должен быть в кодировке UTF-8 без BOM
StrUtf8 read_all_text(const StrUtf8& path);

} // namespace dviglo
