// Copyright (c) the Dviglo project
// License: MIT

#include "shader_cache.hpp"

#include "../fs/log.hpp"


namespace dviglo
{

ShaderProgram* ShaderCache::get(const fs::path& vertex_shader_path,
                                const fs::path& fragment_shader_path,
                                const fs::path& geometry_shader_path)
{
    StrUtf8 id = vertex_shader_path.lexically_normal().string()
                 + "*" + fragment_shader_path.lexically_normal().string()
                 + "*" + geometry_shader_path.lexically_normal().string();

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
    DV_LOG->write_debug("ShaderCache constructed");
}

ShaderCache::~ShaderCache()
{
    instance_ = nullptr;

    for (auto& it : storage_)
        delete it.second;

    storage_.clear();
    DV_LOG->write_debug("ShaderCache destructed");
}

} // namespace dviglo
