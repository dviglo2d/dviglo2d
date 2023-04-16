// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "shader_program.hpp"

#include <dv_string.hpp>
#include <dv_subsystem.hpp>
#include <unordered_map>


namespace dviglo
{

class ShaderCache final
#ifndef NDEBUG
    : public Subsystem
#endif
{
private:
    // Инициализируется в конструкторе
    inline static ShaderCache* instance_ = nullptr;

    std::unordered_map<fs::path, ShaderProgram*> storage_;

public:
    static ShaderCache* instance() { return instance_; }

    ShaderCache();
    ~ShaderCache();

    ShaderProgram* get(const fs::path& vertex_shader_path,
                       const fs::path& fragment_shader_path,
                       const fs::path& geometry_shader_path = fs::path());
};

#define DV_SHADER_CACHE (dviglo::ShaderCache::instance())

} // namespace dviglo
