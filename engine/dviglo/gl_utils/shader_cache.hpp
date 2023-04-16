// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "shader_program.hpp"

#include <dv_string.hpp>
#include <dv_subsystem_base.hpp>
#include <unordered_map>


namespace dviglo
{

class ShaderCache final : public SubsystemBase<ShaderCache>
{
private:
    std::unordered_map<fs::path, ShaderProgram*> storage_;

public:
    ShaderCache();
    ~ShaderCache();

    ShaderProgram* get(const fs::path& vertex_shader_path,
                       const fs::path& fragment_shader_path,
                       const fs::path& geometry_shader_path = fs::path());
};

} // namespace dviglo

#define DV_SHADER_CACHE (dviglo::ShaderCache::instance())
