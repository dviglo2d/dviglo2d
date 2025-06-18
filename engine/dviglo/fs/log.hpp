// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_fs.hpp>
#include <dv_string.hpp>
#include <fstream>
#include <mutex>

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


// Пока существует объект, вывод в std::cout будет дублироваться в файл
class Log
{
private:
    // Выводит текст сразу в 2 буфера
    class TeeBuffer : public std::streambuf
    {
    private:
        // std::cout.rdbuf(). Не может быть nullptr
        std::streambuf* cout_buf_;

        // Буфер файла может быть nullptr
        std::streambuf* file_buf_;

        std::mutex& mutex_;

    protected:
        i32 overflow(i32 ch = EOF) override;
        std::streamsize xsputn(const char* s, std::streamsize count) override;
        i32 sync() override;

    public:
        TeeBuffer(std::streambuf* cout_buf, std::streambuf* file_buf, std::mutex& mutex);
    };

    // Инициализируется в конструкторе
    inline static Log* instance_ = nullptr;

    // Оригинальный std::cout.rdbuf()
    std::streambuf* cout_buf_orig_;

    // Оригинальный std::cerr.rdbuf()
    std::streambuf* cerr_buf_orig_;

    // Файл лога
    std::ofstream file_stream_;

    // Буфер этого разветвителя заменит буфер std::cout
    TeeBuffer tee_cout_;

    // Буфер этого разветвителя заменит буфер std::cerr
    TeeBuffer tee_cerr_;

    std::mutex mutex_;

public:
    static Log* instance() { return instance_; }

    Log(const fs::path& path);
    ~Log();

    static void write(LogLevel message_type, StrViewUtf8 message);

    static void write_debug(StrViewUtf8 message)
    {
        write(LogLevel::debug, message);
    }

    static void write_info(StrViewUtf8 message)
    {
        write(LogLevel::info, message);
    }

    static void write_warning(StrViewUtf8 message)
    {
        write(LogLevel::warning, message);
    }

    static void write_error(StrViewUtf8 message)
    {
        write(LogLevel::error, message);
    }

    template <typename... Types>
    static void writef_debug(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::debug , std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    static void writef_info(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::info, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    static void writef_warning(const std::format_string<Types...> fmt, Types&&... args)
    {
        write(LogLevel::warning, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Types>
    static void writef_error(const std::format_string<Types...> fmt, Types&&... args)
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
