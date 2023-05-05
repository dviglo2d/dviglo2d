// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../io/log.h"


namespace dviglo
{

class DV_API Application
{
protected:
    // Параметры движка, используемые при инициализации

    std::string log_path_;

    Application();
    ~Application();

public:
    i32 run();
};

} // namespace dviglo
