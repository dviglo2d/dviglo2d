// Copyright (c) the Dviglo project
// License: MIT

#include "texture.hpp"

#include "../fs/log.hpp"

#include <pugixml.hpp>

#include <memory>

using namespace glm;
using namespace pugi;
using namespace std;


namespace dviglo
{

// Пытается загрузить xml-файл с настройками текстуры.
// В случае неудачи возвращает дефолтные параметры
static TextureParams try_load_xml(const StrUtf8& xml_file_path)
{
    TextureParams ret = Texture::default_params;

    xml_document doc;
    xml_parse_result result = doc.load_file(xml_file_path.c_str());

    if (result.status == xml_parse_status::status_file_not_found)
        return ret;

    if (!result)
    {
        DV_LOG->writef_error(R"(load_xml("{}") | !result)", xml_file_path);
        return ret;
    }

    xml_node root_node = doc.first_child();

    if (root_node.name() != string("texture"))
    {
        DV_LOG->writef_error(R"(load_xml("{}") | root_node.name() != string("texture"))", xml_file_path);
        return ret;
    }

    for (xml_node child : root_node)
    {
        StrUtf8 key(child.name());

        if (key == "GL_TEXTURE_MIN_FILTER")
        {
            StrUtf8 value(child.child_value());

            if (value == "GL_NEAREST")
                ret.min_filter = GL_NEAREST;
            else if (value == "GL_LINEAR")
                ret.min_filter = GL_LINEAR;
            else if (value == "GL_NEAREST_MIPMAP_NEAREST")
                ret.min_filter = GL_NEAREST_MIPMAP_NEAREST;
            else if (value == "GL_LINEAR_MIPMAP_NEAREST")
                ret.min_filter = GL_LINEAR_MIPMAP_NEAREST;
            else if (value == "GL_NEAREST_MIPMAP_LINEAR")
                ret.min_filter = GL_NEAREST_MIPMAP_LINEAR;
            else if (value == "GL_LINEAR_MIPMAP_LINEAR")
                ret.min_filter = GL_LINEAR_MIPMAP_LINEAR;
            else
                DV_LOG->writef_error(R"(load_xml("{}") | GL_TEXTURE_MIN_FILTER | incorrect value "{}")", xml_file_path, value);
        }
        else if (key == "GL_TEXTURE_MAG_FILTER")
        {
            StrUtf8 value(child.child_value());

            if (value == "GL_NEAREST")
                ret.mag_filter = GL_NEAREST;
            else if (value == "GL_LINEAR")
                ret.mag_filter = GL_LINEAR;
            else
                DV_LOG->writef_error(R"(load_xml("{}") | GL_TEXTURE_MAG_FILTER | incorrect value "{}")", xml_file_path, value);
        }
        else
        {
            DV_LOG->writef_error(R"(load_xml("{}") | incorrect key "{}")", xml_file_path, key);
        }
    }

    return ret;
}

Texture::Texture(const StrUtf8& file_path)
{
    shared_ptr<Image> image = make_shared<Image>(file_path, true);

    GLenum img_format;

    if (image->num_components() == 4)
    {
        img_format = GL_RGBA;
    }
    else if (image->num_components() == 3)
    {
        img_format = GL_RGB;
    }
    else if (image->num_components() == 2)
    {
        img_format = GL_RG;
    }
    else if (image->num_components() == 1)
    {
        img_format = GL_RED;
    }
    else
    {
        DV_LOG->writef_error("Texture::Texture(shared_ptr<Image>) | image->num_components() == {}", image->num_components());
        from_error_image();
        return;
    }

    size_ = image->size();
    image_ = image;

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size_.x, size_.y, 0, img_format, GL_UNSIGNED_BYTE, image->data());
    glGenerateMipmap(GL_TEXTURE_2D);
    set_params(try_load_xml(file_path + ".xml"));
}

Texture::Texture(ivec2 size)
    : size_(size)
{
    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::Texture(const Image& image)
{
    GLenum img_format;

    if (image.num_components() == 4)
    {
        img_format = GL_RGBA;
    }
    else if (image.num_components() == 3)
    {
        img_format = GL_RGB;
    }
    else if (image.num_components() == 2)
    {
        img_format = GL_RG;
    }
    else if (image.num_components() == 1)
    {
        img_format = GL_RED;
    }
    else
    {
        DV_LOG->writef_error("Texture::Texture(const Image&) | image.num_components() == {}", image.num_components());
        from_error_image();
        return;
    }

    size_ = image.size();

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size_.x, size_.y, 0, img_format, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::Texture(shared_ptr<Image> image, bool keep_ptr)
{
    GLenum img_format;

    if (image->num_components() == 4)
    {
        img_format = GL_RGBA;
    }
    else if (image->num_components() == 3)
    {
        img_format = GL_RGB;
    }
    else if (image->num_components() == 2)
    {
        img_format = GL_RG;
    }
    else if (image->num_components() == 1)
    {
        img_format = GL_RED;
    }
    else
    {
        DV_LOG->writef_error("Texture::Texture(shared_ptr<Image>) | image->num_components() == {}", image->num_components());
        from_error_image();
        return;
    }

    size_ = image->size();

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size_.x, size_.y, 0, img_format, GL_UNSIGNED_BYTE, image->data());
    glGenerateMipmap(GL_TEXTURE_2D);

    if (keep_ptr)
        image_ = image;
}

void Texture::from_error_image()
{
    size_ = error_image.size();

    GLenum img_format = (error_image.num_components() == 3) ? GL_RGB : GL_RGBA;

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size_.x, size_.y, 0, img_format, GL_UNSIGNED_BYTE, error_image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

} // namespace dviglo
