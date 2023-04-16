// Copyright (c) the Dviglo project
// License: MIT

#include "dv_tee_buffer.hpp"

#include <cassert>

using namespace std;


namespace dviglo
{

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

} // namespace dviglo
