// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../base/subsystem.hpp"


namespace dviglo
{

class Audio final
#ifndef NDEBUG
    : public Subsystem
#endif
{
private:
    // Инициализируется в конструкторе
    inline static Audio* instance_ = nullptr;

public:
    static Audio* instance() { return instance_; }

    Audio();
    ~Audio();
};

#define DV_AUDIO (dviglo::Audio::instance())

} // namespace dviglo
