// Copyright (c) the Dviglo project
// License: MIT

#include "audio.hpp"

#include "../fs/log.hpp"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <cassert>


namespace dviglo
{

Audio::Audio()
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        DV_LOG->writef_error("{} | !SDL_InitSubSystem(SDL_INIT_AUDIO) | {}", DV_FUNCSIG, SDL_GetError());
        return;
    }

    i32 audio_volume = MIX_MAX_VOLUME;

    SDL_AudioSpec spec;
    spec.freq = MIX_DEFAULT_FREQUENCY;
    spec.format = MIX_DEFAULT_FORMAT;
    spec.channels = MIX_DEFAULT_CHANNELS;

    if (!Mix_OpenAudio(0, &spec))
    {
        DV_LOG->writef_error("{} | !Mix_OpenAudio(...) | {}", DV_FUNCSIG, SDL_GetError());
        return;
    }

    Mix_QuerySpec(&spec.freq, &spec.format, &spec.channels);

    DV_LOG->writef_info("Opened audio at {} Hz {} bit{} {}",
                        spec.freq,
                        spec.format & 0xFF,
                        SDL_AUDIO_ISFLOAT(spec.format) ? " (float)" : "",
                        (spec.channels > 2) ? "surround" : (spec.channels > 1) ? "stereo" : "mono");

    Mix_VolumeMusic(audio_volume);

    instance_ = this;
    DV_LOG->write_debug("Audio constructed");
}

Audio::~Audio()
{
    instance_ = nullptr;

    Mix_CloseAudio();
    Mix_Quit();

    DV_LOG->write_debug("Audio destructed");
}

} // namespace dviglo
