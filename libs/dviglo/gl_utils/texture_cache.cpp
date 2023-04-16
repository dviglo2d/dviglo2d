// Copyright (c) the Dviglo project
// License: MIT

#include "texture_cache.hpp"

#include "../fs/log.hpp"

#include <cassert>


namespace dviglo
{

TextureCache::TextureCache()
{
    assert(!instance_);
    instance_ = this;
    DV_LOG->write_debug("TextureCache constructed");
}

TextureCache::~TextureCache()
{
    instance_ = nullptr;

    for (auto& it : storage_)
        delete it.second;

    storage_.clear();
    DV_LOG->write_debug("TextureCache destructed");
}

Texture* TextureCache::get(const StrUtf8& file_path)
{
    auto it = storage_.find(file_path);

    if (it != storage_.end())
        return it->second;

    Texture* texture = new Texture(file_path);
    storage_[file_path] = texture;

    return texture;
}

} // namespace dviglo
