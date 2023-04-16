// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem.hpp>

#include <SDL3_mixer/SDL_mixer.h>


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

    MIX_Mixer* mixer_ = nullptr;
    bool mix_inited_ = false;

public:
    static Audio* instance() { return instance_; }

    Audio();
    ~Audio();

    MIX_Mixer* mixer() const { return mixer_; }
};

#define DV_AUDIO (dviglo::Audio::instance())

} // namespace dviglo
