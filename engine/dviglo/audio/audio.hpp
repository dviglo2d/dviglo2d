// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem.hpp>

#include <SDL3_mixer/SDL_mixer.h>


namespace dviglo
{

class Audio final : public Subsystem
{
private:
    // Инициализируется в конструкторе
    inline static Audio* instance_ = nullptr;

    MIX_Mixer* mixer_ = nullptr;
    bool mix_inited_ = false;

public:
    static Audio* instance()
    {
        assert(instance_);
        return instance_;
    }

    Audio();
    ~Audio();

    // Запрещаем копирование
    Audio(const Audio&) = delete;
    Audio& operator =(const Audio&) = delete;

    MIX_Mixer* mixer() const { return mixer_; }
};

} // namespace dviglo

#define DV_AUDIO (dviglo::Audio::instance())
