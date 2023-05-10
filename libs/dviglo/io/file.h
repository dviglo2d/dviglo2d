// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.h"


namespace dviglo
{

/// Читает содержимое всего файла в строку. Файл должен быть в кодировке UTF-8 без BOM
StrUtf8 read_all_text(const StrUtf8& path);

} // namespace dviglo
