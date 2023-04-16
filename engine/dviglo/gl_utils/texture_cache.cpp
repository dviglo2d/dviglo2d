// Copyright (c) the Dviglo project
// License: MIT

#include "texture_cache.hpp"

#include "../fs/log.hpp"

#include <cassert>

using namespace std;


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

    for (const auto& pair : umap_storage_)
    {
        if (pair.second.use_count() != 1)
            DV_LOG->writef_error("TextureCache::~TextureCache() | pair.second.use_count() == {}", pair.second.use_count());
    }

    for (const shared_ptr<Texture>& texture : vec_storage_)
    {
        if (texture.use_count() != 1)
            DV_LOG->writef_error("TextureCache::~TextureCache() | texture.use_count() == {}", texture.use_count());
    }

    umap_storage_.clear();
    vec_storage_.clear();

    DV_LOG->write_debug("TextureCache destructed");
}

std::shared_ptr<Texture> TextureCache::get(const StrUtf8& file_path)
{
    auto it = umap_storage_.find(file_path);

    if (it != umap_storage_.end())
        return it->second;

    shared_ptr<Texture> texture = make_shared<Texture>(file_path);
    umap_storage_[file_path] = texture;

    return texture;
}

void TextureCache::add(std::shared_ptr<Texture> texture)
{
    vec_storage_.push_back(texture);
}

void TextureCache::remove(std::shared_ptr<Texture> texture)
{
    auto it = find(vec_storage_.begin(), vec_storage_.end(), texture);

    if (it == vec_storage_.end())
        return;

    // Заменяем элемент последним
    *it = std::move(vec_storage_.back());
    // Укорачиваем список
    vec_storage_.pop_back();
}

} // namespace dviglo
