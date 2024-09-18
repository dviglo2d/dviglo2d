// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/string.hpp"


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


class Log
{
private:
    /// Инициализируется в конструкторе
    inline static Log* instance_ = nullptr;

    FILE* stream_ = nullptr;

public:
    static Log* instance() { return instance_; }

    Log(const StrUtf8& path);
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
