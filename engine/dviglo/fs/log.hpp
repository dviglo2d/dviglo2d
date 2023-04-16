// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../std_utils/fs.hpp"
#include "../std_utils/string.hpp"

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


class Log
{
private:
    // Инициализируется в конструкторе
    inline static Log* instance_ = nullptr;

    std::ofstream stream_;

public:
    static Log* instance() { return instance_; }

    Log(const fs::path& path);
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

    template <typename... Types>
    void writef_debug(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::debug , std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    void writef_info(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::info, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    void writef_warning(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::warning, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    void writef_error(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::error, std::vformat(fmt.get(), std::make_format_args(args...)));
    }
};

#define DV_LOG (dviglo::Log::instance())

// В MSVC выглядит         void __cdecl App::update(__int64)
// В GCC и MinGW выглядит  void App::update(dvt::i64)
// В Clang выглядит        void App::update(i64)
#if defined(DV_WINDOWS_MSVC)
    #define DV_FUNC_SIG StrUtf8(__FUNCSIG__)
#elif defined(DV_LINUX_GCC) || defined(DV_LINUX_CLANG) || defined(DV_WINDOWS_MINGW)
    #define DV_FUNC_SIG StrUtf8(__PRETTY_FUNCTION__)
#else
    #error
#endif

} // namespace dviglo
