// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "log.h"

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

Log::Log(StrViewUtf8 path)
{
    assert(!log_);

    log_ = this;

    cout << '[' << time_to_str() << "] Конструктор лога: " << path << endl;
}

Log::~Log()
{
    log_ = nullptr;

    cout << '[' << time_to_str() << "] Деструктор лога" << endl;
}

} // namespace dviglo
