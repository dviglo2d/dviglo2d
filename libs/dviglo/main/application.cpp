// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "application.h"

#include "../io/log.h"


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
    new Log(log_path_); // Указатель на лог хранится в Log::log_
    delete Log::log_;
}

} // namespace dviglo
