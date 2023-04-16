// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "shader_program.hpp"

#include <dv_string.hpp>
#include <dv_subsystem.hpp>
#include <unordered_map>


namespace dviglo
{

class ShaderCache final : public Subsystem
{
private:
    // Инициализируется в конструкторе
    inline static ShaderCache* instance_ = nullptr;

    std::unordered_map<fs::path, ShaderProgram*> storage_;

public:
    static ShaderCache* instance()
    {
        assert(instance_);
        return instance_;
    }

    ShaderCache();
    ~ShaderCache();

    // Запрещаем копирование
    ShaderCache(const ShaderCache&) = delete;
    ShaderCache& operator =(const ShaderCache&) = delete;

    ShaderProgram* get(const fs::path& vertex_shader_path,
                       const fs::path& fragment_shader_path,
                       const fs::path& geometry_shader_path = fs::path());
};

} // namespace dviglo

#define DV_SHADER_CACHE (dviglo::ShaderCache::instance())
