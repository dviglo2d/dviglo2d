// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "log.h"

#include <cassert>
#include <iostream>

using namespace std;


namespace dviglo
{

Log::Log(StrViewUtf8 path)
{
    assert(!log_);

    log_ = this;

    cout << "Конструктор лога: " << path << endl;
}

Log::~Log()
{
    log_ = nullptr;

    cout << "Деструктор лога" << endl;
}

} // namespace dviglo
