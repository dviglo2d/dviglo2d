// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#pragma once

#include "../gl_utils/shader_program.h"
#include "../gl_utils/vertex_buffer.h"

#include <glm/glm.hpp>

#include <memory>


namespace dviglo
{

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

        /// Цвет в формате 0xAABBGGRR
        u32 color;
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

    // ============================ Общее ============================

public:
    SpriteBatch(ShaderProgram* t_shader_program);

    /// Рендерит накопленную геометрию (то есть текущую порцию)
    void flush();

    // ======================= Используем пакетный рендеринг треугольников =======================

    void draw_triangle(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2);
};

} // namespace dviglo
