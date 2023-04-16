// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../vulkan/vulkan_utils.hpp"

#include <dv_subsystem_index.hpp>


namespace dviglo
{

class SamplerCache final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе, если не было ошибок
    inline static SamplerCache* instance_ = nullptr;

    // Этот сэмплер возвращается, если что-то идёт не так
    vk::Sampler default_sampler_;

    std::unordered_map<vk::SamplerCreateInfo, vk::Sampler> samplers_;

public:
    static SamplerCache* instance() { return instance_; }

    // Возвращает true в ret_success, если не было ошибок
    SamplerCache(bool& ret_success);

    ~SamplerCache() override;

    // Ищет в кэше сэмплер и создаёт новый, если нужного ещё нет
    [[nodiscard]]
    vk::Sampler get(const vk::SamplerCreateInfo& sampler_create_info);
};

} // namespace dviglo

#define DV_SAMPLER_CACHE (dviglo::SamplerCache::instance())
