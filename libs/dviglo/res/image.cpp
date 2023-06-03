// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>


namespace dviglo
{

Image::Image(const StrUtf8& path)
{
    data_ = reinterpret_cast<byte*>(stbi_load(path.c_str(), &width_, &height_, &num_components_, 0));
}

Image::~Image()
{
    stbi_image_free(data_);
}

} // namespace dviglo
