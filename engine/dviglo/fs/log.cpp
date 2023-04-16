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
{
    assert(!instance_);

    instance_ = this;

    write_debug("Log constructed");

    stream_.open(path);

    if (stream_)
        writef_info("Opened log file {}", path);
    else
        writef_error("Failed to open log file {}", path);
}

Log::~Log()
{
    if (stream_.is_open())
    {
        write_info("Closed log file");
        stream_.close();
    }

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

    StrUtf8 str = format("[{}] {}: {}\n", time_to_str(), to_string(message_type), message);
    cout << str;

    if (stream_)
    {
        stream_ << str;
        stream_.flush();
    }
}

} // namespace dviglo
