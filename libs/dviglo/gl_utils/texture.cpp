// Copyright (c) the Dviglo project
// License: MIT

#include "texture.hpp"

#include "../res/image.hpp"

#include <memory>

using namespace std;


namespace dviglo
{

Texture::Texture(const StrUtf8& file_path)
{
    unique_ptr<Image> image = make_unique<Image>(file_path);

    GLenum format;

    if (image->num_components() == 3)
        format = GL_RGB;
    else
        format = GL_RGBA;

    width_ = image->width();
    height_ = image->height();

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width(), image->height(), 0, format, GL_UNSIGNED_BYTE, image->data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // Включаем трилинейную фильтрацию
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

} // namespace dviglo
