// Copyright (c) the Dviglo project
// License: MIT

#include "texture.hpp"

#include "fbo.hpp"
#include "shader_program.hpp"
#include "vertex_buffer.hpp"

#include <dv_log.hpp>
#include <pugixml.hpp>

#include <memory>

using namespace glm;
using namespace pugi;
using namespace std;


namespace dviglo
{

// Пытается загрузить xml-файл с настройками текстуры.
// В случае неудачи возвращает дефолтные параметры
static TextureParams try_load_xml(const fs::path& path)
{
    TextureParams ret = Texture::default_params;

    xml_document doc;
    xml_parse_result result = doc.load_file(path.c_str());

    if (result.status == xml_parse_status::status_file_not_found)
        return ret;

    if (!result)
    {
        Log::writef_error("{} | {} | !result", DV_FUNC_SIG, path);
        return ret;
    }

    xml_node root_node = doc.first_child();

    if (root_node.name() != string("texture"))
    {
        Log::writef_error(R"({} | {} | root_node.name() != string("texture"))", DV_FUNC_SIG, path);
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
                Log::writef_error("{} | {} | GL_TEXTURE_MIN_FILTER | incorrect value | {}", DV_FUNC_SIG, path, value);
        }
        else if (key == "GL_TEXTURE_MAG_FILTER")
        {
            StrUtf8 value(child.child_value());

            if (value == "GL_NEAREST")
                ret.mag_filter = GL_NEAREST;
            else if (value == "GL_LINEAR")
                ret.mag_filter = GL_LINEAR;
            else
                Log::writef_error("{} | {} | GL_TEXTURE_MAG_FILTER | incorrect value | {}", DV_FUNC_SIG, path, value);
        }
        else
        {
            Log::writef_error("{} | {} | incorrect key | {}", DV_FUNC_SIG, path, key);
        }
    }

    return ret;
}

static constexpr GLenum to_image_format(const i32 num_components)
{
    assert(num_components >= 1 && num_components <= 4);
    constexpr GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
    return formats[num_components - 1];
}

static constexpr GLint to_interal_format(const i32 num_components)
{
    assert(num_components >= 1 && num_components <= 4);
    constexpr GLint formats[] = { GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };
    return formats[num_components - 1];
}

Texture::Texture(const fs::path& path)
{
    shared_ptr<Image> image = make_shared<Image>(path, true);

    // TODO: Пустое изображение надо обрабатывать иначе
    if (image->num_components() < 1 || image->num_components() > 4)
    {
        Log::writef_error("{} | image->num_components() == {}", DV_FUNC_SIG, image->num_components());
        from_error_image(4);
        return;
    }

    size_ = image->size();
    num_components_ = image->num_components();
    image_ = image;

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // https://stackoverflow.com/questions/58925604/glteximage2d-crashing-program
    GLenum image_format = to_image_format(num_components_);
    GLint internal_format = to_interal_format(num_components_);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size_.x, size_.y, 0, image_format, GL_UNSIGNED_BYTE, image->data());
    glGenerateMipmap(GL_TEXTURE_2D);
    set_params(try_load_xml(path / ".xml"));
}

Texture::Texture(const ivec2 size, const i32 num_components)
    : size_(size)
    , num_components_(num_components)
{
    assert(num_components >= 1 && num_components <= 4);

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLint internal_format = to_interal_format(num_components_);
    GLenum image_format = to_image_format(num_components_);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.x, size.y, 0, image_format, GL_UNSIGNED_BYTE, nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::Texture(const Image& image)
{
    // TODO: Пустое изображение надо обрабатывать иначе
    if (image.num_components() < 1 || image.num_components() > 4)
    {
        Log::writef_error("{} | image->num_components() == {}", DV_FUNC_SIG, image.num_components());
        from_error_image(4);
        return;
    }

    size_ = image.size();
    num_components_ = image.num_components();

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLint internal_format = to_interal_format(num_components_);
    GLenum image_format = to_image_format(num_components_);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size_.x, size_.y, 0, image_format, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::Texture(shared_ptr<Image> image, bool keep_ptr)
{
    assert(image);

    // TODO: Пустое изображение надо обрабатывать иначе
    if (image->num_components() < 1 || image->num_components() > 4)
    {
        Log::writef_error("{} | image->num_components() == {}", DV_FUNC_SIG, image->num_components());
        from_error_image(4);
        return;
    }

    size_ = image->size();
    num_components_ = image->num_components();

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLint internal_format = to_interal_format(num_components_);
    GLenum image_format = to_image_format(num_components_);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size_.x, size_.y, 0, image_format, GL_UNSIGNED_BYTE, image->data());
    glGenerateMipmap(GL_TEXTURE_2D);

    if (keep_ptr)
        image_ = image;
}

void Texture::from_error_image(i32 num_component)
{
    assert(num_component >= 1 && num_component <= 4);

    const Image& error_image = get_error_image(num_component);

    size_ = error_image.size();
    num_components_ = error_image.num_components();

    glGenTextures(1, &gpu_object_name_);
    glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLint internal_format = to_interal_format(num_components_);
    GLenum image_format = to_image_format(num_components_);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size_.x, size_.y, 0, image_format, GL_UNSIGNED_BYTE, error_image.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::copy_to_cpu()
{
    if (!image_)
        image_ = make_unique<Image>(size_, num_components_);

    assert(size_ == image_->size());
    assert(num_components_ == image_->num_components());

    bind();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    GLenum image_format = to_image_format(num_components_);
    glGetTexImage(GL_TEXTURE_2D, 0, image_format, GL_UNSIGNED_BYTE, image_->data());
}

void Texture::apply_shader(ShaderProgram* shader_program, function<void()> func)
{
    Fbo fbo(size_, num_components_);
    fbo.bind();

    vec2 vertices[] = { {-1, -1}, {1, -1}, {-1, 1}, {1, 1} };
    VertexBuffer vb(4, VertexAttributes::position, BufferUsage::static_draw, vertices);
    // vb.bind() уже вызван в конструкторе

    bind();

    shader_program->use();
    shader_program->set("u_size", size_);
    shader_program->set("u_inv_size", 1.f / vec2(size_)); // TODO: Деление на 0 проверить?
    shader_program->set("u_num_components", num_components_);

    if (func)
        func();

    glViewport(0, 0, fbo.texture()->size().x, fbo.texture()->size().y);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Заменяем текущую текстуру новой
    *this = std::move(*fbo.move_texture());

    // Обновляем изображение в ОЗУ.
    // Можно вызвать copy_to_cpu(), но у меня glReadPixels(...) работает немного быстрее glGetTexImage(...)
    if (image_)
    {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        GLenum image_format = to_image_format(num_components_);
        glReadPixels(0, 0, size_.x, size_.y, image_format, GL_UNSIGNED_BYTE, image_->data());
    }
}

} // namespace dviglo
