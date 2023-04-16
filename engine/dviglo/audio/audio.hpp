// Copyright (c) the Dviglo project
// License: MIT

#pragma once


namespace dviglo
{

class Audio
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
