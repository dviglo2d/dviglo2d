// Copyright (c) the Dviglo project
// License: MIT

#include "dv_log.hpp"

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

static const StrUtf8& to_string(LogLevel level)
{
    static StrUtf8 strings[]
    {
        "DEBUG",   // 0
        "INFO",    // 1
        "WARNING", // 2

        // С подчёркиваниями, чтобы легче было заметить в консоли Linux среди кучи DEBUG
        "_ERROR_"  // 3
    };

    return strings[(u32)level];
}


Log::Log(const fs::path& path)
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this; // Используется в write(...)

    create_dirs(path.parent_path());
    file_stream_.open(path);

    if (file_stream_.is_open())
        writef_info("Opened log file {}", path);
    else
        writef_error("Failed to open log file {}", path);
}

Log::~Log()
{
    if (file_stream_.is_open())
    {
        write_info("Closed log file"); // Использует instance_
        file_stream_.close();
    }

    instance_ = nullptr;
}

void Log::write(LogLevel message_type, StrViewUtf8 message)
{
    if (message_type == LogLevel::none)
        return;

    string str = format("[{}] {}: {}", time_to_str(), to_string(message_type), message);

    if (message_type != LogLevel::error)
        cout << str << endl;
    else // message_type == LogLevel::error
        cerr << str << endl;

    if (instance_ && instance_->file_stream_.is_open())
        instance_->file_stream_ << str << endl; // std::endl автоматически вызывает flush()
}

} // namespace dviglo
