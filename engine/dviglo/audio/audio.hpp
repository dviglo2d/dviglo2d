// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem_index.hpp>

#include <SDL3_mixer/SDL_mixer.h>


namespace dviglo
{

class Audio final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static Audio* instance_ = nullptr;

    MIX_Mixer* mixer_ = nullptr;
    bool mix_init_called_ = false;

public:
    static Audio* instance() { return instance_; }

    Audio();
    ~Audio() final;

    MIX_Mixer* mixer() const { return mixer_; }
};

} // namespace dviglo

#define DV_AUDIO (dviglo::Audio::instance())
