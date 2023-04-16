// Copyright (c) the Dviglo project
// License: MIT

#include "log.hpp"

#include <cassert>
#include <chrono>
#include <iostream>

using namespace std;


namespace dviglo
{

#if DV_LOG_METHOD != DV_LOG_NONE
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
#endif


#if DV_LOG_METHOD == DV_LOG_TEE_BUFFER
// https://en.cppreference.com/w/cpp/io/basic_streambuf/overflow.html
i32 TeeBuffer::overflow(i32 ch)
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

    return std_buf_->sputc(static_cast<char>(ch));
}

// https://en.cppreference.com/w/cpp/io/basic_streambuf/sputn.html
streamsize TeeBuffer::xsputn(const char* s, streamsize count)
{
    lock_guard lock(mutex_);

    if (file_buf_)
        file_buf_->sputn(s, count);

    return std_buf_->sputn(s, count);
}

// https://en.cppreference.com/w/cpp/io/basic_streambuf/pubsync.html
// Функция должна всегда возвращать 0 (успех), иначе в Windows с DV_WIN32_CONSOLE=0
// в лог будет выведено только первое сообщение
i32 TeeBuffer::sync()
{
    lock_guard lock(mutex_);

    if (file_buf_)
        file_buf_->pubsync();

    std_buf_->pubsync();

    return 0;
}

TeeBuffer::TeeBuffer(streambuf* cout_buf, streambuf* file_buf, mutex& mutex)
    : std_buf_(cout_buf)
    , file_buf_(file_buf)
    , mutex_(mutex)
{
    assert(std_buf_ != nullptr);
}
#endif


Log::Log(const fs::path& path)
#if (DV_LOG_METHOD == DV_LOG_SIMPLE) || (DV_LOG_METHOD == DV_LOG_TEE_BUFFER)
    : file_stream_(path)
#endif
#if DV_LOG_METHOD == DV_LOG_TEE_BUFFER
    , cout_buf_orig_(cout.rdbuf())
    , cerr_buf_orig_(cerr.rdbuf())
    , tee_cout_(cout_buf_orig_, file_stream_.rdbuf(), mutex_)
    , tee_cerr_(cerr_buf_orig_, file_stream_.rdbuf(), mutex_)
#endif
{
    assert(!instance_);

    instance_ = this;

    write_debug("Log constructed");

#if DV_LOG_METHOD == DV_LOG_TEE_BUFFER
    // Заменям буфер std::cout разветвителем
    cout.rdbuf(&tee_cout_);

    // Заменям буфер std::cerr разветвителем
    cerr.rdbuf(&tee_cerr_);
#endif

#if (DV_LOG_METHOD == DV_LOG_NONE) || (DV_LOG_METHOD == DV_LOG_CONSOLE_ONLY)
    (void)path;
#else
    if (file_stream_)
        writef_info("Opened log file {}", path);
    else
        writef_error("Failed to open log file {}", path);
#endif
}

Log::~Log()
{
#if (DV_LOG_METHOD == DV_LOG_SIMPLE) || (DV_LOG_METHOD == DV_LOG_TEE_BUFFER)
    if (file_stream_.is_open())
    {
        write_info("Closed log file");
        file_stream_.close();
    }
#endif

#if DV_LOG_METHOD == DV_LOG_TEE_BUFFER
    // Восстанавливаем оригинальный буфер std::cout
    cout.rdbuf(cout_buf_orig_);

    // Восстанавливаем оригинальный буфер std::cerr
    cerr.rdbuf(cerr_buf_orig_);
#endif

    instance_ = nullptr;

    write_debug("Log destructed");
}

void Log::write(LogLevel message_type, StrViewUtf8 message)
{
#if DV_LOG_METHOD == DV_LOG_NONE
    (void)message_type;
    (void)message;
#else
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

#if DV_LOG_METHOD == DV_LOG_SIMPLE
    // Стандартные потоки вывода не перехватыватся. Пишем в файл сами
    if (file_stream_)
        file_stream_ << str << endl; // std::endl автоматически вызывает flush()
#endif

#endif // DV_LOG_METHOD != DV_LOG_NONE
}

} // namespace dviglo
