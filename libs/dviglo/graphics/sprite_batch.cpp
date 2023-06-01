// Copyright (c) 2022-2023 the Dviglo project
// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include "sprite_batch.h"

#include "../gl_utils/shader_cache.h"
#include "../io/fs_base.h"
#include "../math/math.h"

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

SpriteBatch::SpriteBatch()
{
    t_vertex_buffer_ = make_unique<VertexBuffer>(max_triangles_in_portion_ * vertices_per_triangle_,
        VertexAttributes::position | VertexAttributes::color, BufferUsage::dynamic_draw, nullptr);

    StrUtf8 base_path = get_base_path();
    t_shader_program_ = DV_SHADER_CACHE->get(base_path + "data/shaders/vert_color.vert", base_path + "data/shaders/vert_color.frag");
    q_current_shader_program_ = q_default_shader_program_ = DV_SHADER_CACHE->get(base_path + "data/shaders/vert_color_texture.vert", base_path + "data/shaders/vert_color_texture.frag");
    quad.shader_program = sprite.shader_program = q_default_shader_program_;

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

// ======================= Используем пакетный рендеринг четырёхугольников =======================

void SpriteBatch::draw_sprite_internal()
{
    quad.shader_program = sprite.shader_program;
    quad.texture = sprite.texture;

    // Если спрайт не отмасштабирован и не повёрнут, то прорисовка очень проста
    if (sprite.rotation == 0.f && sprite.scale == vec2(1.f, 1.f))
    {
        // Сдвигаем спрайт на -origin
        Rect result_dest(sprite.destination.min - sprite.origin, sprite.destination.max - sprite.origin);

        // Лицевая грань задаётся против часовой стрелки, ось Y направлена вниз
        quad.v0.position = vec2(result_dest.min.x, result_dest.min.y); // Верхний левый угол спрайта
        quad.v1.position = vec2(result_dest.min.x, result_dest.max.y); // Нижний левый угол
        quad.v2.position = vec2(result_dest.max.x, result_dest.max.y); // Нижний правый угол
        quad.v3.position = vec2(result_dest.max.x, result_dest.min.y); // Верхний правый угол
    }
    else
    {
        // Масштабировать и вращать необходимо относительно центра локальных координат:
        // 1) При стандартном origin == vec2(0.f, 0.f), который соответствует верхнему левому углу спрайта,
        //    локальные координаты будут Rect(ноль, размеры_спрайта),
        //    то есть Rect(ноль, destination.max - destination.min)
        // 2) При ненулевом origin нужно сдвинуть на -origin
        Rect local(-sprite.origin, sprite.destination.max - sprite.destination.min - sprite.origin);

        float sin, cos;
        sin_cos(sprite.rotation, sin, cos);

        // Нам нужна матрица, которая масштабирует и поворачивает вершину в локальных координатах, а затем
        // смещает ее в требуемые мировые координаты.
        // Но в матрице 3x3 последняя строка "0 0 1", умножать на которую бессмысленно.
        // Поэтому вычисляем без матрицы для оптимизации
        float m11 =  cos * sprite.scale.x; float m12 = sin * sprite.scale.y; float m13 = sprite.destination.min.x;
        float m21 = -sin * sprite.scale.x; float m22 = cos * sprite.scale.y; float m23 = sprite.destination.min.y;
        //          0                                  0                                 1

        float min_x_m11 = local.min.x * m11;
        float min_x_m21 = local.min.x * m21;
        float max_x_m11 = local.max.x * m11;
        float max_x_m21 = local.max.x * m21;
        float min_y_m12 = local.min.y * m12;
        float min_y_m22 = local.min.y * m22;
        float max_y_m12 = local.max.y * m12;
        float max_y_m22 = local.max.y * m22;

        // transform * vec2(local.min.x, local.min.y)
        quad.v0.position = vec2(min_x_m11 + min_y_m12 + m13,
                                min_x_m21 + min_y_m22 + m23);

        // transform * vec2(local.min.x, local.max.y)
        quad.v1.position = vec2(min_x_m11 + max_y_m12 + m13,
                                min_x_m21 + max_y_m22 + m23);

        // transform * vec2(local.max.x, local.max.y)
        quad.v2.position = vec2(max_x_m11 + max_y_m12 + m13,
                                max_x_m21 + max_y_m22 + m23);

        // transform * vec2(local.max.x, local.min.y)
        quad.v3.position = vec2(max_x_m11 + min_y_m12 + m13,
                                max_x_m21 + min_y_m22 + m23);
    }

    if (!!(sprite.flip_modes & FlipModes::horizontally))
        swap(sprite.source_uv.min.x, sprite.source_uv.max.x);

    if (!!(sprite.flip_modes & FlipModes::vertically))
        swap(sprite.source_uv.min.y, sprite.source_uv.max.y);

    quad.v0.color = sprite.color0;
    quad.v0.uv = sprite.source_uv.min;

    quad.v1.color = sprite.color1;
    quad.v1.uv = vec2(sprite.source_uv.min.x, sprite.source_uv.max.y);

    quad.v2.color = sprite.color2;
    quad.v2.uv = sprite.source_uv.max;

    quad.v3.color = sprite.color3;
    quad.v3.uv = vec2(sprite.source_uv.max.x, sprite.source_uv.min.y);

    add_quad();
}

// Преобразует пиксельные координаты в диапазон [0, 1]
static Rect SrcToUV(const Rect* source, Texture* texture)
{
    if (source == nullptr)
    {
        return Rect(vec2(0.f, 0.f), vec2(1.f, 1.f));
    }
    else
    {
        // Проверки не производятся, текстура должна быть корректной
        float inv_width = 1.f / texture->width();
        float inv_height = 1.f / texture->height();

        return Rect
        (
            {source->min.x * inv_width, source->min.y * inv_height},
            {source->max.x * inv_width, source->max.y * inv_height}
        );
    }
}

static Rect PosToDest(const vec2& position, Texture* texture, const Rect* src)
{
    if (src == nullptr)
    {
        // Проверки не производятся, текстура должна быть корректной
        return Rect
        (
            {position.x, position.y},
            {position.x + texture->width(), position.y + texture->height()}
        );
    }
    else
    {
        return Rect
        (
            {position.x, position.y},
            {position.x + (src->max.x - src->min.x), // Сперва вычисляем размер, так как там вероятно более близкие
            position.y + (src->max.y - src->min.y)} // значения и меньше ошибка вычислений
        );
    }
}

void SpriteBatch::draw_sprite(Texture* texture, const Rect& destination, const Rect* source, u32 color,
    float rotation, const vec2& origin, const vec2& scale, FlipModes flip_modes)
{
    if (!texture)
        return;

    sprite.texture = texture;
    sprite.shader_program = q_default_shader_program_;
    sprite.destination = destination;
    sprite.source_uv = SrcToUV(source, texture);
    sprite.flip_modes = flip_modes;
    sprite.scale = scale;
    sprite.rotation = rotation;
    sprite.origin = origin;
    sprite.color0 = color;
    sprite.color1 = color;
    sprite.color2 = color;
    sprite.color3 = color;

    draw_sprite_internal();
}

void SpriteBatch::draw_sprite(Texture* texture, const vec2& position, const Rect* source, u32 color,
    float rotation, const vec2& origin, const vec2& scale, FlipModes flip_modes)
{
    if (!texture)
        return;

    sprite.texture = texture;
    sprite.shader_program = q_default_shader_program_;
    sprite.destination = PosToDest(position, texture, source);
    sprite.source_uv = SrcToUV(source, texture);
    sprite.flip_modes = flip_modes;
    sprite.scale = scale;
    sprite.rotation = rotation;
    sprite.origin = origin;
    sprite.color0 = color;
    sprite.color1 = color;
    sprite.color2 = color;
    sprite.color3 = color;

    draw_sprite_internal();
}

void SpriteBatch::draw_string(const StrUtf8& text, SpriteFont* font, const vec2& position, u32 color,
    float rotation, const vec2& origin, const vec2& scale, FlipModes flip_modes)
{
    if (text.length() == 0)
        return;

    vector<c32> unicode_text;
    size_t offset = 0;
    while (offset < text.length())
    {
        c32 code_point = next_code_point(text, offset);
        unicode_text.push_back(code_point);
    }

    sprite.shader_program = q_default_shader_program_;
    sprite.flip_modes = flip_modes;
    sprite.scale = scale;
    sprite.rotation = rotation;
    sprite.color0 = color;
    sprite.color1 = color;
    sprite.color2 = color;
    sprite.color3 = color;
    sprite.texture = &font->texture(0);

    // По идее все текстуры одинакового размера
    float pixel_width = 1.f / sprite.texture->width();
    float pixel_height = 1.f / sprite.texture->height();

    vec2 char_pos = position;
    vec2 char_orig = origin;

    i32 i = 0;
    i32 step = 1;

    if (!!(flip_modes & FlipModes::horizontally))
    {
        i = unicode_text.size() - 1;
        step = -1;
    }

    for (; i >= 0 && i < unicode_text.size(); i += step)
    {
        const Glyph& glyph = font->glyph(unicode_text[i]);

        float gx = (float)glyph.x;
        float gy = (float)glyph.y;
        float gw = (float)glyph.width;
        float gh = (float)glyph.height;
        float gox = (float)glyph.offset_x;
        float goy = (float)glyph.offset_y;

        sprite.texture = &font->texture(glyph.page);
        sprite.destination = Rect({char_pos.x, char_pos.y}, {char_pos.x + gw, char_pos.y + gh});
        sprite.source_uv = Rect({gx * pixel_width, gy * pixel_height}, {(gx + gw) * pixel_width, (gy + gh) * pixel_height});

        // Модифицируем origin, а не позицию, чтобы было правильное вращение
        sprite.origin = !!(flip_modes & FlipModes::vertically) ? char_orig - vec2(gox, font->line_height() - goy - gh) : char_orig - vec2(gox, goy);

        draw_sprite_internal();

        char_orig.x -= (float)glyph.advance_x;
    }
}

} // namespace dviglo
