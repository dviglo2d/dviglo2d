// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_fs.hpp"
#include "dv_meta.hpp"
#include "dv_subsystem_index.hpp"

#include <fstream>


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


// В лог можно писать не создавая экземпляр класса. При этом текст будет выведен в консоль, но не в файл.
// Класс не наследуется от SubsystemBase, так как SubsystemBase использует лог
class Log final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static Log* instance_ = nullptr;

    // Файл лога
    std::ofstream file_stream_;

public:
    Log(const fs::path& path);
    ~Log();

    static void write(LogLevel message_type, StrViewUtf8 message);

    inline static void write_debug(StrViewUtf8 message)
    {
        write(LogLevel::debug, message);
    }

    inline static void write_info(StrViewUtf8 message)
    {
        write(LogLevel::info, message);
    }

    inline static void write_warning(StrViewUtf8 message)
    {
        write(LogLevel::warning, message);
    }

    inline static void write_error(StrViewUtf8 message)
    {
        write(LogLevel::error, message);
    }

    template <typename... Types>
    inline static void writef_debug(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::debug , std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    inline static void writef_info(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::info, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    inline static void writef_warning(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::warning, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    inline static void writef_error(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::error, std::vformat(fmt.get(), std::make_format_args(args...)));
    }
};

} // namespace dviglo
