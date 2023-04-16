// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem_base.hpp>

#include <SDL3_mixer/SDL_mixer.h>


namespace dviglo
{

class Audio final : public SubsystemBase<Audio>
{
private:
    MIX_Mixer* mixer_ = nullptr;
    bool mix_inited_ = false;

public:
    Audio();
    ~Audio();

    MIX_Mixer* mixer() const { return mixer_; }
};

} // namespace dviglo

#define DV_AUDIO (dviglo::Audio::instance())
