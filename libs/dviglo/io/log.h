// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.h"


namespace dviglo
{

class DV_API Log
{
private:
    /// Инициализируется в конструкторе
    inline static Log* log_ = nullptr;

public:
    static Log* instance() { return log_; }

    Log(StrViewUtf8 path);
    ~Log();
};

} // namespace dviglo
