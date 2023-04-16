// Copyright (c) the Dviglo project
// License: MIT

#include "sampler_cache.hpp"

#include "../main/graphics.hpp"

#include <dv_log.hpp>


namespace dviglo
{

SamplerCache::SamplerCache(bool& ret_success)
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    vk::SamplerCreateInfo default_sampler_create_info
    {
        .magFilter = vk::Filter::eNearest,
        .minFilter = vk::Filter::eNearest,
        .mipmapMode = vk::SamplerMipmapMode::eNearest,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
    };

    vk::Result vk_result;
    std::tie(vk_result, default_sampler_) = DV_GRAPHICS->device().createSampler(default_sampler_create_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | DV_GRAPHICS->device().createSampler(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        ret_success = false;
        return;
    }

    samplers_[default_sampler_create_info] = default_sampler_;

    ret_success = true;
}

SamplerCache::~SamplerCache()
{
    instance_ = nullptr;


}

vk::Sampler SamplerCache::get(const vk::SamplerCreateInfo& sampler_create_info)
{
    return default_sampler_;
}

} // namespace dviglo
