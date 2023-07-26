// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "audio.hpp"

#include "../io/log.hpp"

#include <fmt/format.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include <cassert>

using namespace fmt;


namespace dviglo
{

Audio::Audio()
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    {
        DV_LOG->write_error(format("Audio::Audio(): SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 | {}", SDL_GetError()));
        return;
    }

    i32 audio_rate = MIX_DEFAULT_FREQUENCY;
    Uint16 audio_format = MIX_DEFAULT_FORMAT;
    i32 audio_channels = MIX_DEFAULT_CHANNELS;
    i32 audio_buffers = 4096;
    i32 audio_volume = MIX_MAX_VOLUME;

    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0)
    {
        DV_LOG->write_error(format("Audio::Audio(): Mix_OpenAudio(...) < 0 | {}", SDL_GetError()));
        return;
    }

    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

    DV_LOG->write_info(format(
        "Opened audio at {} Hz {} bit{} {} {} bytes audio buffer\n",
        audio_rate,
        audio_format & 0xFF,
        SDL_AUDIO_ISFLOAT(audio_format) ? " (float)" : "",
        (audio_channels > 2) ? "surround" : (audio_channels > 1) ? "stereo" : "mono",
        audio_buffers
    ));

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
