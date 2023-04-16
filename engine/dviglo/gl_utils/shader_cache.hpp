// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "shader_program.hpp"

#include <dv_string.hpp>
#include <dv_subsystem_index.hpp>
#include <unordered_map>


namespace dviglo
{

class ShaderCache final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static ShaderCache* instance_ = nullptr;

    std::unordered_map<fs::path, ShaderProgram*> storage_;

public:
    static ShaderCache* instance() { return instance_; }

    ShaderCache();
    ~ShaderCache() final;

    ShaderProgram* get(const fs::path& vertex_shader_path,
                       const fs::path& fragment_shader_path,
                       const fs::path& geometry_shader_path = fs::path());
};

} // namespace dviglo

#define DV_SHADER_CACHE (dviglo::ShaderCache::instance())
