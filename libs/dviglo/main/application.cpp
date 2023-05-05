// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "application.h"

#include "../io/log.h"

#include <memory>

using namespace std;


namespace dviglo
{

Application::Application()
{
}

Application::~Application()
{
}

void Application::run()
{
    unique_ptr<Log> log = make_unique<Log>(log_path_);
}

} // namespace dviglo
