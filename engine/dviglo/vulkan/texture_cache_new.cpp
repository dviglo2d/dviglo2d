// Copyright (c) the Dviglo project
// License: MIT

#include "texture_cache_new.hpp"


namespace dviglo
{

TextureCacheNew::TextureCacheNew()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;
}

TextureCacheNew::~TextureCacheNew()
{
    instance_ = nullptr;
}

} // namespace dviglo
