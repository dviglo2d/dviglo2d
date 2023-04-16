// Copyright (c) the Dviglo project
// License: MIT

#include "file_base.hpp"
#include "log.hpp"

#include <cassert>
#include <iostream>

using namespace std;


namespace dviglo
{

static StrUtf8 time_to_str()
{
    time_t current_time = time(nullptr);
    char tmp_buffer[sizeof "yyyy-mm-dd hh:mm:ss"];

    // %F и %T не работают в MinGW
    //strftime(tmp_buffer, sizeof tmp_buffer, "%F %T", localtime(&current_time));

    strftime(tmp_buffer, sizeof tmp_buffer, "%Y-%m-%d %H:%M:%S", localtime(&current_time));
    return StrUtf8(tmp_buffer);
}

Log::Log(const StrUtf8& path)
{
    assert(!instance_);

    instance_ = this;

    write_debug("Log constructed");

    stream_ = file_open(path.c_str(), "w");

    if (stream_)
        writef_info("Opened log file {}", path);
    else
        writef_error("Failed to open log file {}", path);
}

Log::~Log()
{
    write_info("Closed log file");

    if (stream_)
    {
        file_close(stream_);
        stream_ = nullptr;
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

        // С восклицательными знаками, чтобы легче было заметить в консоли Linux среди кучи DEBUG
        "!!ERROR"  // 3
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
        file_write(str.c_str(), 1, (i32)str.size(), stream_);
        file_flush(stream_);
    }
}

} // namespace dviglo
