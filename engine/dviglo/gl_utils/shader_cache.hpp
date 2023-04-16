// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "shader_program.hpp"

#include "../std_utils/string.hpp"

#include <unordered_map>


namespace dviglo
{

class ShaderCache
{
private:
    // Инициализируется в конструкторе
    inline static ShaderCache* instance_ = nullptr;

    std::unordered_map<StrUtf8, ShaderProgram*> storage_;

public:
    static ShaderCache* instance() { return instance_; }

    ShaderCache();
    ~ShaderCache();

    ShaderProgram* get(const StrUtf8& vertex_shader_path, const StrUtf8& fragment_shader_path,
                       const StrUtf8& geometry_shader_path = StrUtf8());
};

#define DV_SHADER_CACHE (dviglo::ShaderCache::instance())

} // namespace dviglo
