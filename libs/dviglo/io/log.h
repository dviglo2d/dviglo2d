// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/str.h"


namespace dviglo
{

enum class LogLevel : u32
{
    debug = 0, // 0
    info,      // 1
    warning,   // 2
    error,     // 3
    none       // 4
};

class DV_API Log
{
private:
    /// Инициализируется в конструкторе
    inline static Log* instance_ = nullptr;

public:
    static Log* instance() { return instance_; }

    Log(StrViewUtf8 path);
    ~Log();

    void write(LogLevel message_type, StrViewUtf8 message);

    void write_debug(StrViewUtf8 message)
    {
        write(LogLevel::debug, message);
    }

    void write_info(StrViewUtf8 message)
    {
        write(LogLevel::info, message);
    }

    void write_warning(StrViewUtf8 message)
    {
        write(LogLevel::warning, message);
    }

    void write_error(StrViewUtf8 message)
    {
        write(LogLevel::error, message);
    }
};

#define DV_LOG (dviglo::Log::instance())

} // namespace dviglo
