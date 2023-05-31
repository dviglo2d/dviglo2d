// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#pragma once

#include "../gl_utils/index_buffer.h"
#include "../gl_utils/shader_program.h"
#include "../gl_utils/texture.h"
#include "../gl_utils/vertex_buffer.h"
#include "../math/rect.h"

#include <memory>


namespace dviglo
{

/// Режимы зеркального отображения спрайтов и текста
enum class FlipModes : u32
{
    none         = 0,
    horizontally = 1 << 0,
    vertically   = 1 << 1,
    both = horizontally | vertically
};
DV_FLAGS(FlipModes);


class DV_API SpriteBatch
{
    // ============================ Пакетный рендеринг треугольников ============================

private:

    /// Максимальное число треугольников в порции
    inline static constexpr i32 max_triangles_in_portion_ = 512;

    /// Число вершин в треугольнике
    inline static constexpr i32 vertices_per_triangle_ = 3;

    /// Атрибуты вершин треугольников
    struct TVertex
    {
        glm::vec2 position;
        u32 color; ///< Цвет в формате 0xAABBGGRR
    };

    /// Текущая порция треугольников
    TVertex t_vertices_[max_triangles_in_portion_ * vertices_per_triangle_];

    /// Число вершин в массиве t_vertices_
    i32 t_num_vertices_ = 0;

    /// Шейдерная программа для рендеринга треугольников
    ShaderProgram* t_shader_program_;

    /// Вершинный буфер для треугольников (индексный буфер не используется)
    std::unique_ptr<VertexBuffer> t_vertex_buffer_;

public:

    /// Данные для функции add_triangle().
    /// Заполняем заранее выделенную память, вместо передачи кучи аргументов в функцию
    struct
    {
        TVertex v0, v1, v2;
    } triangle_;

    /// Добавляет 3 вершины в массив t_vertices_. Вызывает flush(), если массив полон.
    /// Перед вызовом этой функции необходимо заполнить структуру triangle_
    void add_triangle();

    /// Указывает цвет для следующих треугольников (в формате 0xAABBGGRR)
    void set_shape_color(u32 color);

    // ============================ Пакетный рендеринг четырёхугольников ============================

private:

    /// Максимальное число четырёхугольников в порции
    inline static constexpr i32 max_quads_in_portion_ = 500;

    /// Четырёхугольник состоит из двух треугольников, а значит у него 6 вершин.
    /// То есть каждый четырёхугольник занимает 6 элементов в индексном буфере
    inline static constexpr i32 indices_per_quad_ = 6;

    /// Две вершины четырёхугольника идентичны для обоих треугольников, поэтому
    /// в вершинном буфере каждый четырёхугольник занимает 4 элемента
    inline static constexpr u16 vertices_per_quad_ = 4;

    /// Атрибуты вершин четырёхугольников
    struct QVertex
    {
        glm::vec2 position;
        u32 color; ///< Цвет в формате 0xAABBGGRR
        glm::vec2 uv;
    };

    /// Текущая порция четырёхугольников
    QVertex q_vertices_[max_quads_in_portion_ * vertices_per_quad_];

    /// Число вершин в массиве q_vertices_
    i32 q_num_vertices_ = 0;

    /// Текущая текстура для четырёхугольников
    Texture* q_current_texture_ = nullptr;

    /// Дефолтная шейдерная программа для четырёхугольников
    ShaderProgram* q_default_shader_program_;

    /// Текущая шейдерная программа для четырёхугольников
    ShaderProgram* q_current_shader_program_;

    /// Вершинный буфер для четырёхугольников
    std::unique_ptr<VertexBuffer> q_vertex_buffer_;

    /// Индексный буфер для четырёхугольников
    std::unique_ptr<IndexBuffer> q_index_buffer_;

public:

    /// Данные для функции add_quad().
    /// Заполняем заранее выделенную память, вместо передачи кучи аргументов в функцию
    struct
    {
        Texture* texture;
        ShaderProgram* shader_program;
        QVertex v0, v1, v2, v3;
    } quad;

    /// Добавляет 4 вершины в массив q_vertices_.
    /// Если массив полон или требуемые шейдеры или текстура отличаются от текущих, то автоматически
    /// происходит вызов функции flush() (то есть начинается новая порция).
    /// Перед вызовом этой функции необходимо заполнить структуру quad
    void add_quad();

    // ============================ Общее ============================

public:

    SpriteBatch();

    /// Рендерит накопленную геометрию (то есть текущую порцию)
    void flush();

    // ======================= Используем пакетный рендеринг треугольников =======================

    void draw_triangle(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2);

    // ======================= Используем пакетный рендеринг четырёхугольников =======================

private:

    /// Данные для функции draw_sprite()
    struct
    {
        Texture* texture;
        ShaderProgram* shader_program;
        Rect destination;
        Rect source_uv; ///< Текстурные координаты в диапазоне [0, 1]
        FlipModes flip_modes;
        glm::vec2 scale;
        float rotation;
        glm::vec2 origin;
        u32 color0; ///< Левый верхний угол
        u32 color1; ///< Левый нижний угол
        u32 color2; ///< Правый нижний угол
        u32 color3; ///< Правый верхний угол
    } sprite;

    /// Перед вызовом этой функции нужно заполнить структуру sprite. Функция может изменить данные в структуре
    void draw_sprite_internal();

public:

    /// color - цвет в формате 0xAABBGGRR
    void draw_sprite(Texture* texture, const Rect& destination, const Rect* source = nullptr, u32 color = 0xFFFFFFFF,
        float rotation = 0.f, const glm::vec2& origin = {0.f, 0.f}, const glm::vec2& scale = {1.f, 1.f}, FlipModes flip_modes = FlipModes::none);

    /// color - цвет в формате 0xAABBGGRR
    void draw_sprite(Texture* texture, const glm::vec2& position, const Rect* source = nullptr, u32 color = 0xFFFFFFFF,
        float rotation = 0.f, const glm::vec2& origin = {0.f, 0.f}, const glm::vec2& scale = {1.f, 1.f}, FlipModes flip_modes = FlipModes::none);
};

} // namespace dviglo
