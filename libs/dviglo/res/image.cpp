// Copyright (c) the Dviglo project
// License: MIT

#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

using namespace glm;


namespace dviglo
{

Image::Image()
    : size_(0, 0)
    , num_components_(0)
    , data_(nullptr)
{
}

Image::Image(ivec2 size, i32 num_components)
    : size_(size)
    , num_components_(num_components)
{
    data_ = reinterpret_cast<byte*>(STBI_MALLOC(size_.x * size_.y * num_components_));
}

Image::~Image()
{
    stbi_image_free(data_);
}

Image Image::from_file(const StrUtf8& path)
{
    Image ret;
    ret.data_ = reinterpret_cast<byte*>(stbi_load(path.c_str(), &ret.size_.x, &ret.size_.y, &ret.num_components_, 0));
    return ret;
}

} // namespace dviglo
