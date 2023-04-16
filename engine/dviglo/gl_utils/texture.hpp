// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../res/image.hpp"

#include <glad/gl.h>

#include <memory>  // shared_ptr
#include <utility> // std::exchange()


namespace dviglo
{

class ShaderProgram;

struct TextureParams
{
    GLint min_filter = GL_NEAREST_MIPMAP_LINEAR;
    GLint mag_filter = GL_LINEAR;
};

class Texture
{
private:
    // Идентификатор объекта OpenGL
    GLuint gpu_object_name_;

    // Размер
    glm::ivec2 size_;

    // Число цветовых каналов.
    // Передаётся в фильтр
    i32 num_components_;

    // Копия текстуры в ОЗУ.
    // Можно не хранить
    std::shared_ptr<Image> image_;

    // Если что-то пошло не так, то используем шахматную текстуру
    void from_error_image(i32 num_component = 4);

public:
    // Эти параметры используются по умолчанию при создании новой текстуры
    inline static TextureParams default_params;

    Texture()
        : gpu_object_name_(0)
        , size_{}
        , num_components_(0)
    {
    }

    // Загружает тестуру из файла
    Texture(const fs::path& path);

    // Создаёт пустую RGBA текстуру нужного размера
    Texture(const glm::ivec2 size, const i32 num_components = 4);

    // Создаёт текстуру из изображения
    Texture(const Image& image);

    // Создаёт текстуру из изображения. Опционально может сохранить указатель на изображение
    Texture(std::shared_ptr<Image> image, bool keep_ptr = false);

    ~Texture()
    {
        glDeleteTextures(1, &gpu_object_name_); // Проверка на 0 не нужна
        gpu_object_name_ = 0;
    }

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут хранить уничтоженный gpu_object_name_
    Texture(const Texture&) = delete;
    Texture& operator =(const Texture&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе

    Texture(Texture&& other) noexcept
        : gpu_object_name_(std::exchange(other.gpu_object_name_, 0))
        , size_(std::exchange(other.size_, {}))
        , num_components_(std::exchange(other.num_components_, 0))
    {
    }

    Texture& operator =(Texture&& other) noexcept
    {
        if (this != &other)
        {
            gpu_object_name_ = std::exchange(other.gpu_object_name_, 0);
            size_ = std::exchange(other.size_, {});
            num_components_ = std::exchange(other.num_components_, 0);
        }

        return *this;
    }

    glm::ivec2 size() const { return size_; }
    i32 width() const { return size_.x; }
    i32 height() const { return size_.y; }
    GLuint gpu_object_name() const { return gpu_object_name_; }
    std::shared_ptr<Image> image() const { return image_; }

    void bind()
    {
        glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
    }

    void set_params(const TextureParams& params)
    {
        glBindTexture(GL_TEXTURE_2D, gpu_object_name_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.mag_filter);
    }

    // Копирует текстуру из видеопамяти в ОЗУ (в image_)
    void copy_to_cpu();

    // Применяет фильтр к текстуре. Функция func позволяет установить
    // дополнительные uniform-ы перед рендерингом
    void apply_shader(ShaderProgram* shader_program, std::function<void()> func = nullptr);
};

} // namespace dviglo
