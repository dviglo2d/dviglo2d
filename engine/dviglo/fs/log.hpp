// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../base/subsystem.hpp"

#include <dv_fs.hpp>
#include <dv_string.hpp>
#include <fstream>
#include <mutex>


// +++++ Метод журналирования +++++

// Не выводим ни в консоль, ни в файл
#define DV_LOG_NONE 0

// Выводим только в std::cout и в std::cerr, но не в файл
#define DV_LOG_CONSOLE_ONLY 1

// Выводим в std::cout, в std::cerr и в std::ofstream file_stream_
#define DV_LOG_SIMPLE 2

// Выводим в std::cout и в std::cerr.
// TeeBuffer перехватывает вывод в std::cout и std::cerr и дублирует его в файл.
// Это позволяет сохранять в файл даже вывод сторонних библиотек.
// printf(...) и fprintf(stderr, ...) не перехватываются
#define DV_LOG_TEE_BUFFER 3

// Выбираем один из методов журналирования
#define DV_LOG_METHOD DV_LOG_TEE_BUFFER

// ----- Метод журналирования -----


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


#if DV_LOG_METHOD == DV_LOG_TEE_BUFFER
// Выводит текст сразу в 2 буфера
class TeeBuffer : public std::streambuf
{
private:
    // Оригинальный std::cout.rdbuf() или std::cerr.rdbuf(). Не может быть nullptr
    std::streambuf* std_buf_;

    // file_stream.rdbuf(). Может быть nullptr.
    // file_stream - объект типа std::ofstream
    std::streambuf* file_buf_;

    std::mutex& mutex_;

protected:
    i32 overflow(i32 ch = EOF) override;
    std::streamsize xsputn(const char* s, std::streamsize count) override;
    i32 sync() override;

public:
    TeeBuffer(std::streambuf* std_buf, std::streambuf* file_buf, std::mutex& mutex);
};
#endif


class Log final
#ifndef NDEBUG
    : public Subsystem
#endif
{
private:
    // Инициализируется в конструкторе
    inline static Log* instance_ = nullptr;

#if (DV_LOG_METHOD == DV_LOG_SIMPLE) || (DV_LOG_METHOD == DV_LOG_TEE_BUFFER)
    // Файл лога
    std::ofstream file_stream_;
#endif

#if DV_LOG_METHOD == DV_LOG_TEE_BUFFER
    // Оригинальный std::cout.rdbuf()
    std::streambuf* cout_buf_orig_;

    // Оригинальный std::cerr.rdbuf()
    std::streambuf* cerr_buf_orig_;

    // Буфер этого разветвителя заменит буфер std::cout
    TeeBuffer tee_cout_;

    // Буфер этого разветвителя заменит буфер std::cerr
    TeeBuffer tee_cerr_;

    std::mutex mutex_;
#endif

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
