// Copyright (c) the Dviglo project
// License: MIT

#include "log.hpp"

#include <cassert>
#include <chrono>
#include <iostream>

using namespace std;


namespace dviglo
{

// https://en.cppreference.com/w/cpp/io/basic_streambuf/overflow.html
i32 Log::TeeBuffer::overflow(i32 ch)
{
    // https://en.cppreference.com/w/cpp/string/char_traits.html
    static_assert(is_same_v<char_type, char>);

    // Тип, который умещает все символы + EOF
    // https://en.cppreference.com/w/cpp/named_req/CharTraits.html
    static_assert(is_same_v<int_type, i32>);

    // https://en.cppreference.com/w/cpp/string/char_traits/eof.html
    static_assert(traits_type::eof() == EOF);
    static_assert(traits_type::not_eof(EOF) == !EOF);

    if (ch == EOF)
        return !EOF;

    lock_guard lock(mutex_);

    if (file_buf_)
        file_buf_->sputc(static_cast<char>(ch));

    return cout_buf_->sputc(static_cast<char>(ch));
}

// https://en.cppreference.com/w/cpp/io/basic_streambuf/sputn.html
streamsize Log::TeeBuffer::xsputn(const char* s, streamsize count)
{
    lock_guard lock(mutex_);

    if (file_buf_)
        file_buf_->sputn(s, count);

    return cout_buf_->sputn(s, count);
}

// https://en.cppreference.com/w/cpp/io/basic_streambuf/pubsync.html
// Функция должна всегда возвращать 0 (успех), иначе в Windows без консоли
// в лог будет выведено только первое сообщение
i32 Log::TeeBuffer::sync()
{
    lock_guard lock(mutex_);

    if (file_buf_)
        file_buf_->pubsync();

    cout_buf_->pubsync();

    return 0; 
}

Log::TeeBuffer::TeeBuffer(streambuf* cout_buf, streambuf* file_buf, mutex& mutex)
    : cout_buf_(cout_buf)
    , file_buf_(file_buf)
    , mutex_(mutex)
{
    assert(cout_buf_ != nullptr);
}

static StrUtf8 time_to_str()
{
    // Текущее время, округлённое до секунд
    chrono::sys_seconds now_sec = chrono::floor<chrono::seconds>(chrono::system_clock::now());

    // Переводим в локальный часовой пояс
    chrono::zoned_time local_time(chrono::current_zone(), now_sec);

    return format("{:%F %T}", local_time);
}

Log::Log(const fs::path& path)
    : cout_buf_orig_(cout.rdbuf())
    , cerr_buf_orig_(cerr.rdbuf())
    , file_stream_(path)
    , tee_cout_(cout_buf_orig_, file_stream_.rdbuf(), mutex_)
    , tee_cerr_(cerr_buf_orig_, file_stream_.rdbuf(), mutex_)
{
    assert(!instance_);

    instance_ = this;

    write_debug("Log constructed");

    // Заменям буфер std::cout разветвителем
    cout.rdbuf(&tee_cout_);

    // Заменям буфер std::cerr разветвителем
    cerr.rdbuf(&tee_cerr_);

    if (file_stream_)
        writef_info("Opened log file {}", path);
    else
        writef_error("Failed to open log file {}", path);
}

Log::~Log()
{
    if (file_stream_.is_open())
    {
        write_info("Closed log file");
        file_stream_.close();
    }

    // Восстанавливаем оригинальный буфер std::cout
    std::cout.rdbuf(cout_buf_orig_);

    // Восстанавливаем оригинальный буфер std::cerr
    std::cerr.rdbuf(cerr_buf_orig_);

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

    std::string str = format("[{}] {}: {}", time_to_str(), to_string(message_type), message);

    if (message_type == LogLevel::error)
        cerr << str << endl; // endl автоматически вызывает flush()
    else
        cout << str << endl;
}

} // namespace dviglo
