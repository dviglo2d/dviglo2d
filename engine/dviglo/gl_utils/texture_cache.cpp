// Copyright (c) the Dviglo project
// License: MIT

#include "texture_cache.hpp"

#include <dv_log.hpp>

using namespace std;


namespace dviglo
{

TextureCache::TextureCache()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;
}

TextureCache::~TextureCache()
{
    instance_ = nullptr;

    for (const auto& pair : umap_storage_)
    {
        if (pair.second.use_count() != 1)
            Log::writef_error("{} | pair.second.use_count() == {}", DV_FUNC_SIG, pair.second.use_count());
    }

    for (const shared_ptr<Texture>& texture : vec_storage_)
    {
        if (texture.use_count() != 1)
            Log::writef_error("{} | texture.use_count() == {}", DV_FUNC_SIG, texture.use_count());
    }

    umap_storage_.clear();
    vec_storage_.clear();
}

std::shared_ptr<Texture> TextureCache::get(const fs::path& path)
{
    fs::path normal_path = path.lexically_normal();
    auto it = umap_storage_.find(normal_path);

    if (it != umap_storage_.end())
        return it->second;

    shared_ptr<Texture> texture = make_shared<Texture>(normal_path);
    umap_storage_[normal_path] = texture;

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
