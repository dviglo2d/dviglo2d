// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "texture.h"

#include <unordered_map>


namespace dviglo
{

class DV_API TextureCache
{
private:
    /// Инициализируется в конструкторе
    inline static TextureCache* instance_ = nullptr;

    std::unordered_map<StrUtf8, Texture*> storage_;

public:
    static TextureCache* instance() { return instance_; }

    TextureCache();
    ~TextureCache();

    Texture* get(const StrUtf8& file_path);
};

#define DV_TEXTURE_CACHE (dviglo::TextureCache::instance())

} // namespace dviglo
