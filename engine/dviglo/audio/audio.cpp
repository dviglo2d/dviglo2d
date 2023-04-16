// Copyright (c) the Dviglo project
// License: MIT

#include "audio.hpp"

#include <dv_log.hpp>


namespace dviglo
{

Audio::Audio()
{
    if (!SDL_Init(SDL_INIT_AUDIO))
    {
        Log::writef_error("{} | !SDL_Init(SDL_INIT_AUDIO) | {}", DV_FUNC_SIG, SDL_GetError());
        goto end;
    }

    if (!MIX_Init())
    {
        Log::writef_error("{} | !MIX_Init() | {}", DV_FUNC_SIG, SDL_GetError());
        goto end;
    }
    else
    {
        mix_inited_ = true;
    }

    mixer_ = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixer_)
    {
        Log::writef_error("{} | !mixer_ | {}", DV_FUNC_SIG, SDL_GetError());
        goto end;
    }

    {
        SDL_AudioSpec spec;
        MIX_GetMixerFormat(mixer_, &spec);
        Log::writef_info("Mixer is format {}, {} channel{}, {} frequency",
                         SDL_GetAudioFormatName(spec.format),
                         spec.channels,
                         (spec.channels != 1) ? "s" : "",
                         spec.freq);
    }

end:
    instance_ = this;
    Log::write_debug("Audio constructed");
}

Audio::~Audio()
{
    instance_ = nullptr;

    if (mix_inited_)
        MIX_Quit();

    Log::write_debug("Audio destructed");
}

} // namespace dviglo
