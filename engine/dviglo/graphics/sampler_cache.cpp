// Copyright (c) the Dviglo project
// License: MIT

#include "sampler_cache.hpp"

#include "../main/graphics.hpp"

#include <dv_log.hpp>

#include <ranges>


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
        .minLod = 0.f,
        .maxLod = vk::LodClampNone,
    };

    vk::Result vk_result;
    std::tie(vk_result, default_sampler_) = DV_GRAPHICS->device().createSampler(default_sampler_create_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | DV_GRAPHICS->device().createSampler(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        ret_success = false;
        return;
    }

    samplers_.emplace(default_sampler_create_info, default_sampler_);

    ret_success = true;
}

SamplerCache::~SamplerCache()
{
    instance_ = nullptr;

    vk::Result vk_result = DV_GRAPHICS->device().waitIdle();

    if (vk_result != vk::Result::eSuccess)
        Log::writef_error("{} | DV_GRAPHICS->device().waitIdle() | {}", DV_FUNC_SIG, vk::to_string(vk_result));

    for (vk::Sampler sampler : samplers_ | std::views::values)
    {
        // default_sampler_ может содержаться в таблице несколько раз
        if (sampler != default_sampler_)
            DV_GRAPHICS->device().destroySampler(sampler);
    }

    DV_GRAPHICS->device().destroySampler(default_sampler_);
}

vk::Sampler SamplerCache::get(const vk::SamplerCreateInfo& sampler_create_info)
{
    // Если в кэше уже есть сэмплер, то возвращаем его
    if (const auto it = samplers_.find(sampler_create_info); it != samplers_.end())
        return it->second;

    // Создаём новый сэмрлер
    auto [vk_result, new_sampler] = DV_GRAPHICS->device().createSampler(sampler_create_info);

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | DV_GRAPHICS->device().createSampler(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));

        // Не спамим одну и ту же ошибку в лог
        samplers_.emplace(sampler_create_info, default_sampler_);

        return default_sampler_;
    }

    samplers_.emplace(sampler_create_info, new_sampler);

    return new_sampler;
}

} // namespace dviglo
