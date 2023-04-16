// Copyright (c) the Dviglo project
// License: MIT

#include "dv_locale.hpp"

#include "dv_platform.hpp"

#if DV_WINDOWS
    #include <filesystem> // std::filesystem::path
#endif


namespace dviglo
{

bool set_utf8_locale()
{
#if DV_WINDOWS
    bool ret = (std::setlocale(LC_CTYPE, ".UTF-8") != nullptr);

    // Дополнительная проверка на всякий случай
    if (ret)
        ret = (std::filesystem::path("тест").native() == L"тест");
#elif DV_LINUX
    bool ret = (std::setlocale(LC_CTYPE, "C.UTF-8") != nullptr);
#endif

    return ret;
}

} // namespace dviglo
