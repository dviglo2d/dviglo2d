// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_primitive_types.hpp"

#include <mutex>
#include <streambuf>


namespace dviglo
{

// Выводит текст сразу в 2 буфера. Используется, чтобы текст,
// попадающий в std::cout и std::cerr, сохранялся также в файл.
// Не перехватывает текст, выводимый с помощью printf(...) и fprintf(stderr, ...)
class TeeBuffer : public std::streambuf
{
private:
    // std::cout.rdbuf() или std::cerr.rdbuf(). Не может быть nullptr
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

} // namespace dviglo
