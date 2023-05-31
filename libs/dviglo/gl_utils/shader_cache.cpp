// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "shader_cache.h"


namespace dviglo
{

ShaderProgram* ShaderCache::get(const StrUtf8& vertex_shader_path, const StrUtf8& fragment_shader_path,
                                const StrUtf8& geometry_shader_path)
{
    StrUtf8 id = vertex_shader_path + "*" + fragment_shader_path + "*" + geometry_shader_path;

    auto it = storage_.find(id);

    if (it != storage_.end())
        return it->second;

    ShaderProgram* shader_program = new ShaderProgram(vertex_shader_path, fragment_shader_path, geometry_shader_path);
    storage_[id] = shader_program;

    return shader_program;
}

ShaderCache::ShaderCache()
{
    assert(!instance_);
    instance_ = this;
}

ShaderCache::~ShaderCache()
{
    for (auto& it : storage_)
        delete it.second;
}

} // namespace dviglo
