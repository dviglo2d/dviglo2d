// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem_index.hpp>


namespace dviglo
{

// Все текстуры должны храниться в кэше, чтобы гарантировать их уничтожение
// перед уничтожением vk::Device
class TextureCacheNew final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static TextureCacheNew* instance_ = nullptr;

public:
    static TextureCacheNew* instance() { return instance_; }

    TextureCacheNew();
    ~TextureCacheNew() override;
};


} // namespace dviglo

#define DV_TEXTURE_CACHE_NEW (dviglo::TextureCacheNew::instance())
