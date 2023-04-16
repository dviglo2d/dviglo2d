// Copyright (c) the Dviglo project
// License: MIT

#include "log.hpp"

#include <cassert>
#include <chrono>
#include <iostream>

using namespace std;


namespace dviglo
{

static StrUtf8 time_to_str()
{
    // Текущее время, округлённое до секунд
    chrono::sys_seconds now_sec = chrono::floor<chrono::seconds>(chrono::system_clock::now());

    // Переводим в локальный часовой пояс
    chrono::zoned_time local_time(chrono::current_zone(), now_sec);

    return format("{:%F %T}", local_time);
}

Log::Log(const fs::path& path)
#if DV_USE_TEE_BUFFER
    : cout_buf_orig_(cout.rdbuf())
    , cerr_buf_orig_(cerr.rdbuf())
    , file_stream_(path)
    , tee_cout_(cout_buf_orig_, file_stream_.rdbuf(), mutex_)
    , tee_cerr_(cerr_buf_orig_, file_stream_.rdbuf(), mutex_)
#endif
{
    assert(!instance_);

    instance_ = this;

    write_debug("Log constructed");

#if DV_USE_TEE_BUFFER
    // Заменям буфер std::cout разветвителем
    cout.rdbuf(&tee_cout_);

    // Заменям буфер std::cerr разветвителем
    cerr.rdbuf(&tee_cerr_);

    if (file_stream_)
        writef_info("Opened log file {}", path);
    else
        writef_error("Failed to open log file {}", path);
#else
    (void)path;
#endif
}

Log::~Log()
{
#if DV_USE_TEE_BUFFER
    if (file_stream_.is_open())
    {
        write_info("Closed log file");
        file_stream_.close();
    }

    // Восстанавливаем оригинальный буфер std::cout
    cout.rdbuf(cout_buf_orig_);

    // Восстанавливаем оригинальный буфер std::cerr
    cerr.rdbuf(cerr_buf_orig_);
#endif

    instance_ = nullptr;

    write_debug("Log destructed");
}

static const StrUtf8& to_string(LogLevel level)
{
    static StrUtf8 strings[]{
        "DEBUG",   // 0
        "INFO",    // 1
        "WARNING", // 2

        // С подчёркиваниями, чтобы легче было заметить в консоли Linux среди кучи DEBUG
        "_ERROR_"  // 3
    };

    return strings[(u32)level];
}

void Log::write(LogLevel message_type, StrViewUtf8 message)
{
    if (message_type == LogLevel::none)
        return;

    string str = format("[{}] {}: {}", time_to_str(), to_string(message_type), message);

    if (message_type != LogLevel::error)
    {
        // std::endl автоматически вызывает flush().
        // При этом данные передаются дальше в stdout
        // https://en.cppreference.com/w/cpp/io/cout.html
        // https://en.cppreference.com/w/cpp/io/manip/endl
        cout << str << endl;
    }
    else // message_type == LogLevel::error
    {
        // При любом выводе в std::cerr автоматически вызывается flush(), так как установлен флаг unitbuf.
        // При этом данные передаются дальше в stderr.
        // std::clog тоже передаёт данные в stderr, но у него не установлен флаг unitbuf
        // https://en.cppreference.com/w/cpp/io/cerr.html
        // https://en.cppreference.com/w/cpp/io/clog.html
        // https://en.cppreference.com/w/cpp/io/ios_base/flags.html
        cerr << str << endl;
    }
}

} // namespace dviglo
