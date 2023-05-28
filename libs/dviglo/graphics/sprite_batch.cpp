// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "sprite_batch.h"

#include "../io/fs_base.h"

#include <cstring> // memcpy

using namespace glm;
using namespace std;


namespace dviglo
{

void SpriteBatch::add_triangle()
{
    // Рендерили четырёхугольники, а теперь нужно рендерить треугольники
    if (q_num_vertices_ > 0)
        flush();

    memcpy(t_vertices_ + t_num_vertices_, &triangle_, sizeof(triangle_));
    t_num_vertices_ += vertices_per_triangle_;

    // Если после добавления вершин мы заполнили массив до предела, то рендерим порцию
    if (t_num_vertices_ == max_triangles_in_portion_ * vertices_per_triangle_)
        flush();
}

void SpriteBatch::set_shape_color(u32 color)
{
    triangle_.v0.color = color;
    triangle_.v1.color = color;
    triangle_.v2.color = color;
}

void SpriteBatch::add_quad()
{
    // Рендерили треугольники, а теперь нужно рендерить четырёхугольники
    if (t_num_vertices_ > 0)
        flush();

    if (quad.texture != q_current_texture_ || quad.shader_program != q_current_shader_program_)
    {
        flush();

        q_current_texture_ = quad.texture;
        q_current_shader_program_ = quad.shader_program;
    }

    memcpy(q_vertices_ + q_num_vertices_, &(quad.v0), sizeof(QVertex) * vertices_per_quad_);
    q_num_vertices_ += vertices_per_quad_;

    // Если после добавления вершин мы заполнили массив до предела, то рендерим порцию
    if (q_num_vertices_ == max_quads_in_portion_ * vertices_per_quad_)
        flush();
}

SpriteBatch::SpriteBatch(ShaderProgram* t_shader_program)
{
    t_vertex_buffer_ = make_unique<VertexBuffer>(max_triangles_in_portion_ * vertices_per_triangle_,
        VertexAttributes::position | VertexAttributes::color, BufferUsage::dynamic_draw, nullptr);

    t_shader_program_ = t_shader_program;
    set_shape_color(0xFFFFFFFF);

    q_vertex_buffer_ = make_unique<VertexBuffer>(max_quads_in_portion_ * vertices_per_quad_,
        VertexAttributes::position | VertexAttributes::color | VertexAttributes::uv, BufferUsage::dynamic_draw, nullptr);

    // Индексный буфер всегда содержит набор четырёхугольников, поэтому его можно сразу заполнить

    unique_ptr<u16[]> indices = make_unique<u16[]>(max_quads_in_portion_ * indices_per_quad_);

    for (u16 i = 0; i < max_quads_in_portion_; i++)
    {
        // Первый треугольник четырёхугольника
        indices[i * indices_per_quad_ + 0] = i * vertices_per_quad_ + 0;
        indices[i * indices_per_quad_ + 1] = i * vertices_per_quad_ + 1;
        indices[i * indices_per_quad_ + 2] = i * vertices_per_quad_ + 2;

        // Второй треугольник
        indices[i * indices_per_quad_ + 3] = i * vertices_per_quad_ + 2;
        indices[i * indices_per_quad_ + 4] = i * vertices_per_quad_ + 3;
        indices[i * indices_per_quad_ + 5] = i * vertices_per_quad_ + 0;
    }

    q_index_buffer_ = make_unique<IndexBuffer>(max_quads_in_portion_ * indices_per_quad_, IndexType::u16,
                                               BufferUsage::static_draw, indices.get());
}

ivec2 screen_size{800, 600};

void SpriteBatch::flush()
{
    if (t_num_vertices_ > 0)
    {
        t_shader_program_->use();
        t_shader_program_->set("u_scale", vec2(2.f / screen_size.x, -2.f / screen_size.y));

        t_vertex_buffer_->set_data(t_num_vertices_, t_vertices_);

        // t_vertex_buffer_->bind() вызывается в t_vertex_buffer_->set_data()
        glDrawArrays(GL_TRIANGLES, 0, t_vertex_buffer_->num_vertices());

        // Начинаем новую порцию
        t_num_vertices_ = 0;
    }
    else if (q_num_vertices_ > 0)
    {
        q_current_shader_program_->use();
        q_current_shader_program_->set("u_scale", vec2(2.f / screen_size.x, -2.f / screen_size.y));

        glActiveTexture(GL_TEXTURE0);
        q_current_texture_->bind();
        q_current_shader_program_->set("u_texture", 0);

        q_vertex_buffer_->set_data(q_num_vertices_, q_vertices_);

        q_index_buffer_->bind();
        // q_vertex_buffer_->bind() вызывается в q_vertex_buffer_->set_data()
        i32 num_quads = q_num_vertices_ / vertices_per_quad_;
        glDrawElements(GL_TRIANGLES, num_quads * indices_per_quad_, q_index_buffer_->type(), nullptr);

        // Начинаем новую порцию
        q_num_vertices_ = 0;
    }
}

void SpriteBatch::draw_triangle(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2)
{
    triangle_.v0.position = v0;
    triangle_.v1.position = v1;
    triangle_.v2.position = v2;
    add_triangle();
}

} // namespace dviglo
