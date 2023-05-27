// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "sprite_batch.h"

#include "../io/fs_base.h"

#include <cstring> // memcpy

using namespace std;


namespace dviglo
{

void SpriteBatch::add_triangle()
{
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

SpriteBatch::SpriteBatch(ShaderProgram* t_shader_program)
{
    t_vertex_buffer_ = make_unique<VertexBuffer>(max_triangles_in_portion_ * vertices_per_triangle_,
        VertexAttributes::position | VertexAttributes::color, BufferUsage::dynamic_draw, nullptr);

    t_shader_program_ = t_shader_program;
    set_shape_color(0xFFFFFFFF);
}

void SpriteBatch::flush()
{
    if (t_num_vertices_ > 0)
    {
        t_shader_program_->use();
        t_vertex_buffer_->set_data(t_num_vertices_, t_vertices_);

        // t_vertex_buffer_->bind() вызывается в t_vertex_buffer_->set_data()
        glDrawArrays(GL_TRIANGLES, 0, t_vertex_buffer_->num_vertices());

        // Начинаем новую порцию
        t_num_vertices_ = 0;
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
