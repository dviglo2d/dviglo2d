// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.h"


namespace dviglo
{

class DV_API Log
{
    /// Только Engine может создать и уничтожить лог
    friend class Engine;

private:
    /// Инициализируется в конструкторе
    inline static Log* log_ = nullptr;

public:
    Log* get_instance() const { return log_; }

private:
    Log(StrViewUtf8 path);
    ~Log();

public:
};

} // namespace dviglo
