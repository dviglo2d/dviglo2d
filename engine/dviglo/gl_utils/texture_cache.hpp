// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "texture.hpp"

#include <unordered_map>


namespace dviglo
{

// Все текстуры должны храниться в кэше, чтобы гарантировать их уничтожение
// перед уничтожением контекста OpenGL
class TextureCache
{
private:
    // Инициализируется в конструкторе
    inline static TextureCache* instance_ = nullptr;

    std::unordered_map<StrUtf8, std::shared_ptr<Texture>> umap_storage_;
    std::vector<std::shared_ptr<Texture>> vec_storage_;

public:
    static TextureCache* instance() { return instance_; }

    TextureCache();
    ~TextureCache();

    std::shared_ptr<Texture> get(const StrUtf8& file_path);

    // Добавляет текстуру в vec_storage_
    void add(std::shared_ptr<Texture> texture);

    // Убирает текстуру из vec_storage_. Не вызывает деструктор.
    // Меняет порядок элементов в vec_storage_
    void remove(std::shared_ptr<Texture> texture);
};

#define DV_TEXTURE_CACHE (dviglo::TextureCache::instance())

} // namespace dviglo
