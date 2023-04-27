// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "engine.h"

#include "../io/log.h"


namespace dviglo
{

Engine::Engine()
{
    new Log("путь");
}

Engine::~Engine()
{
    delete Log::log_;
}

} // namespace dviglo
