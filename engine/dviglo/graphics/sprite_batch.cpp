// Copyright (c) the Dviglo project
// License: MIT

#include "sprite_batch.hpp"

#include "../fs/fs_base.hpp"
#include "../gl_utils/gl_utils.hpp"
#include "../gl_utils/shader_cache.hpp"
#include "../math/math.hpp"

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

void SpriteBatch::prepare_ogl(bool alpha_blending, bool flip_vertically)
{
    flush(); // На случай, если параметры меняются посередине рендеринга

    flip_vertically_ = flip_vertically;

#if false // Для тестов
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    if (flip_vertically)
        glFrontFace(GL_CCW);
    else
        glFrontFace(GL_CW); // Задаём треугольники по часовой стрелке
#endif

    // Включаем альфа-смешение, если нужно
    if (alpha_blending)
    {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

SpriteBatch::SpriteBatch()
{
    t_vertex_buffer_ = make_unique<VertexBuffer>(max_triangles_in_portion_ * vertices_per_triangle_,
        VertexAttributes::position | VertexAttributes::color, BufferUsage::dynamic_draw, nullptr);

    StrUtf8 base_path = get_base_path();
    t_shader_program_ = DV_SHADER_CACHE->get(base_path + "engine_data/shaders/vert_color.vert", base_path + "engine_data/shaders/vert_color.frag");
    q_current_shader_program_ = q_default_shader_program_ = DV_SHADER_CACHE->get(base_path + "engine_data/shaders/vert_color_texture.vert", base_path + "engine_data/shaders/vert_color_texture.frag");
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

void SpriteBatch::flush()
{
    if (t_num_vertices_ > 0)
    {
        t_shader_program_->use();
        ivec2 viewport_size = get_viewport().size;
        t_shader_program_->set("u_pixel_size", vec2(2.f / viewport_size.x, 2.f / viewport_size.y));
        t_shader_program_->set("u_flip_vertically", flip_vertically_);

        t_vertex_buffer_->set_data(t_num_vertices_, t_vertices_);

        // t_vertex_buffer_->bind() вызывается в t_vertex_buffer_->set_data()
        glDrawArrays(GL_TRIANGLES, 0, t_vertex_buffer_->num_vertices());

        // Начинаем новую порцию
        t_num_vertices_ = 0;
    }
    else if (q_num_vertices_ > 0)
    {
        q_current_shader_program_->use();
        ivec2 viewport_size = get_viewport().size;
        t_shader_program_->set("u_pixel_size", vec2(2.f / viewport_size.x, 2.f / viewport_size.y));
        t_shader_program_->set("u_flip_vertically", flip_vertically_);

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

// ======================= Используем пакетный рендеринг треугольников =======================

void SpriteBatch::draw_triangle(vec2 v0, vec2 v1, vec2 v2)
{
    triangle_.v0.position = v0;
    triangle_.v1.position = v1;
    triangle_.v2.position = v2;
    add_triangle();
}

void SpriteBatch::draw_rect(const Rect& rect)
{
    vec2 far_corner = rect.pos + rect.size;

    triangle_.v0.position = {rect.pos.x, rect.pos.y};
    triangle_.v1.position = {far_corner.x, rect.pos.y};
    triangle_.v2.position = {rect.pos.x, far_corner.y};
    add_triangle();

    triangle_.v0.position = {far_corner.x, rect.pos.y};
    triangle_.v1.position = {far_corner.x, far_corner.y};
    triangle_.v2.position = {rect.pos.x, far_corner.y};
    add_triangle();
}

void SpriteBatch::draw_disk(vec2 center_pos, f32 radius, i32 num_segments)
{
    vector<vec2> points(num_segments);

    for (i32 i = 0; i < num_segments; ++i)
    {
        // Угол увеличивается по часовой стрелке
        f32 angle = radians(360.f) * i / num_segments;
        f32 cos, sin;
        sin_cos(angle, sin, cos);
        points[i] = vec2(cos, sin) * radius + center_pos;
    }

    for (i32 i = 0; i < num_segments - 1; ++i)
        draw_triangle(points[i], points[i + 1], center_pos);

    // Рисуем последний сегмент
    draw_triangle(points[num_segments - 1], points[0], center_pos);
}

// ======================= Используем пакетный рендеринг четырёхугольников =======================

void SpriteBatch::transform_sprite_internal()
{
        // Если спрайт не отмасштабирован и не повёрнут, то прорисовка очень проста
    if (sprite.rotation == 0.f && sprite.scale == vec2(1.f, 1.f))
    {
        // Сдвигаем спрайт на -origin
        Rect result_dest{sprite.destination.pos - sprite.origin, sprite.destination.size};
        vec2 far_corner = result_dest.pos + result_dest.size;

        // Лицевая грань задаётся по часовой стрелке, ось Y направлена вниз
        quad.v0.position = vec2(result_dest.pos.x, result_dest.pos.y); // Верхний левый угол спрайта
        quad.v1.position = vec2(far_corner.x, result_dest.pos.y); // Верхний правый угол
        quad.v2.position = vec2(far_corner.x, far_corner.y); // Нижний правый угол
        quad.v3.position = vec2(result_dest.pos.x, far_corner.y); // Нижний левый угол
    }
    else
    {
        // Масштабировать и вращать необходимо относительно центра локальных координат:
        // 1) При стандартном origin == vec2(0.f, 0.f), который соответствует верхнему левому углу спрайта,
        //    локальные координаты углов будут ноль и размеры_спрайта
        // 2) При ненулевом origin координаты нужно сдвинуть на -origin
        Rect local(-sprite.origin, sprite.destination.size);
        vec2 far_corner = local.pos + local.size;

        f32 sin, cos;
        sin_cos(sprite.rotation, sin, cos);

        // Нам нужна матрица, которая масштабирует и поворачивает вершину в локальных координатах, а затем
        // смещает ее в требуемые мировые координаты.
        // Но в матрице 3x3 последняя строка "0 0 1", умножать на которую бессмысленно.
        // Поэтому вычисляем без матрицы для оптимизации
        f32 m11 = cos * sprite.scale.x; f32 m12 = -sin * sprite.scale.y; f32 m13 = sprite.destination.pos.x;
        f32 m21 = sin * sprite.scale.x; f32 m22 =  cos * sprite.scale.y; f32 m23 = sprite.destination.pos.y;
        //          0                                  0                                 1

        f32 pos_x_m11 = local.pos.x * m11;
        f32 pos_x_m21 = local.pos.x * m21;
        f32 far_x_m11 = far_corner.x * m11;
        f32 far_x_m21 = far_corner.x * m21;
        f32 pos_y_m12 = local.pos.y * m12;
        f32 pos_y_m22 = local.pos.y * m22;
        f32 far_y_m12 = far_corner.y * m12;
        f32 far_y_m22 = far_corner.y * m22;

        // transform * vec2(local.pos.x, local.pos.y)
        quad.v0.position = vec2(pos_x_m11 + pos_y_m12 + m13,
                                pos_x_m21 + pos_y_m22 + m23);

        // transform * vec2(far_corner.x, local.pos.y)
        quad.v1.position = vec2(far_x_m11 + pos_y_m12 + m13,
                                far_x_m21 + pos_y_m22 + m23);

        // transform * vec2(far_corner.x, far_corner.y)
        quad.v2.position = vec2(far_x_m11 + far_y_m12 + m13,
                                far_x_m21 + far_y_m22 + m23);

        // transform * vec2(local.pos.x, far_corner.y)
        quad.v3.position = vec2(pos_x_m11 + far_y_m12 + m13,
                                pos_x_m21 + far_y_m22 + m23);
    }
}

void SpriteBatch::draw_sprite_internal()
{
    quad.shader_program = sprite.shader_program;
    quad.texture = sprite.texture;

    // Вычисляем координаты вершин
    transform_sprite_internal();

    if (!!(sprite.flip_modes & FlipModes::horizontally))
    {
        sprite.source_uv.pos.x += sprite.source_uv.size.x;
        sprite.source_uv.size.x = -sprite.source_uv.size.x;
    }

    if (!!(sprite.flip_modes & FlipModes::vertically))
    {
        sprite.source_uv.pos.y += sprite.source_uv.size.y;
        sprite.source_uv.size.y = -sprite.source_uv.size.y;
    }

    quad.v0.color = sprite.color0;
    quad.v0.uv = sprite.source_uv.pos;

    quad.v1.color = sprite.color1;
    quad.v1.uv = vec2(sprite.source_uv.pos.x + sprite.source_uv.size.x, sprite.source_uv.pos.y);

    quad.v2.color = sprite.color2;
    quad.v2.uv = sprite.source_uv.pos + sprite.source_uv.size;

    quad.v3.color = sprite.color3;
    quad.v3.uv = vec2(sprite.source_uv.pos.x, sprite.source_uv.pos.y + sprite.source_uv.size.y);

    add_quad();
}

Aabb SpriteBatch::measure_sprite_internal()
{
    // Вычисляем координаты вершин
    transform_sprite_internal();

    Aabb ret(quad.v0.position, quad.v0.position);
    ret.merge(quad.v1.position);
    ret.merge(quad.v2.position);
    ret.merge(quad.v3.position);

    return ret;
}

// Преобразует пиксельные координаты в диапазон [0, 1]
static Rect src_to_uv(const Rect* source, Texture* texture)
{
    if (source == nullptr)
    {
        return Rect(0.f, 0.f, 1.f, 1.f);
    }
    else
    {
        // Проверки не производятся, текстура должна быть корректной
        f32 inv_width = 1.f / texture->width();
        f32 inv_height = 1.f / texture->height();

        return Rect
        (
            source->pos.x * inv_width, source->pos.y * inv_height,
            source->size.x * inv_width, source->size.y * inv_height
        );
    }
}

static Rect pos_to_dest(vec2 position, Texture* texture, const Rect* src)
{
    if (src == nullptr)
    {
        // Проверки не производятся, текстура должна быть корректной
        return Rect{position, texture->size()};
    }
    else
    {
        return Rect{position, src->size};
    }
}

void SpriteBatch::draw_sprite(Texture* texture, const Rect& destination, const Rect* source, u32 color,
    f32 rotation, vec2 origin, vec2 scale, FlipModes flip_modes)
{
    if (!texture)
        return;

    sprite.texture = texture;
    sprite.shader_program = q_default_shader_program_;
    sprite.destination = destination;
    sprite.source_uv = src_to_uv(source, texture);
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

Rect SpriteBatch::measure_sprite(const Rect& destination,
    f32 rotation, vec2 origin, vec2 scale)
{
    //sprite.texture = texture;
    //sprite.shader_program = q_default_shader_program_;
    sprite.destination = destination;
    //sprite.source_uv = src_to_uv(source, texture);
    //sprite.flip_modes = flip_modes;
    sprite.scale = scale;
    sprite.rotation = rotation;
    sprite.origin = origin;
    //sprite.color0 = color;
    //sprite.color1 = color;
    //sprite.color2 = color;
    //sprite.color3 = color;

    return measure_sprite_internal().to_rect();
}

void SpriteBatch::draw_sprite(Texture* texture, vec2 position, const Rect* source, u32 color,
    f32 rotation, vec2 origin, vec2 scale, FlipModes flip_modes)
{
    if (!texture)
        return;

    sprite.texture = texture;
    sprite.shader_program = q_default_shader_program_;
    sprite.destination = pos_to_dest(position, texture, source);
    sprite.source_uv = src_to_uv(source, texture);
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

Rect SpriteBatch::measure_sprite(Texture* texture, vec2 position, const Rect* source,
    f32 rotation, vec2 origin, vec2 scale)
{
    if (!texture && !source) // Размер определить невозможно
        return Rect(position, vec2(0.f, 0.f));

    //sprite.texture = texture;
    //sprite.shader_program = q_default_shader_program_;
    sprite.destination = pos_to_dest(position, texture, source);
    //sprite.source_uv = src_to_uv(source, texture);
    //sprite.flip_modes = flip_modes;
    sprite.scale = scale;
    sprite.rotation = rotation;
    sprite.origin = origin;
    //sprite.color0 = color;
    //sprite.color1 = color;
    //sprite.color2 = color;
    //sprite.color3 = color;

    return measure_sprite_internal().to_rect();
}

void SpriteBatch::draw_string(const StrUtf8& text, SpriteFont* font, vec2 position, u32 color,
    f32 rotation, vec2 origin, vec2 scale, FlipModes flip_modes)
{
    if (text.length() == 0)
        return;

    vector<c32> unicode_text = to_utf32(text);

    sprite.shader_program = q_default_shader_program_;
    sprite.flip_modes = flip_modes;
    sprite.scale = scale;
    sprite.rotation = rotation;
    sprite.color0 = color;
    sprite.color1 = color;
    sprite.color2 = color;
    sprite.color3 = color;
    sprite.texture = font->textures()[0].get();

    // По идее все текстуры одинакового размера
    vec2 pixel_size(1.f / sprite.texture->width(), 1.f / sprite.texture->height());

    vec2 char_pos = position;
    vec2 char_orig = origin;

    i32 i = 0;
    i32 step = 1;

    if (!!(flip_modes & FlipModes::horizontally))
    {
        i = (i32)unicode_text.size() - 1;
        step = -1;
    }

    for (; i >= 0 && i < (i32)unicode_text.size(); i += step)
    {
        auto it = font->glyphs().find(unicode_text[i]);

        if (it == font->glyphs().end())
            it = font->glyphs().find('?'); // Вопрос всегда должен быть в шрифте

        const Glyph& glyph = it->second;

        Rect rect(glyph.rect);
        vec2 offset(glyph.offset);

        sprite.texture = font->textures()[glyph.page].get();
        sprite.destination = Rect(char_pos, rect.size);
        sprite.source_uv = Rect(rect.pos * pixel_size, rect.size * pixel_size);

        // Модифицируем origin, а не позицию, чтобы было правильное вращение
        sprite.origin = !!(flip_modes & FlipModes::vertically) ? char_orig - vec2(offset.x, font->line_height() - offset.y - rect.size.y) : char_orig - offset;

        draw_sprite_internal();

        char_orig.x -= (f32)glyph.advance_x;
    }
}

Rect SpriteBatch::measure_string(const StrUtf8& text, SpriteFont* font, vec2 position,
    f32 rotation, vec2 origin, vec2 scale, FlipModes flip_modes)
{
    if (text.length() == 0)
        return Rect::zero; // TODO: Позицию вычислить

    vector<c32> unicode_text = to_utf32(text);

    //sprite.shader_program = q_default_shader_program_;
    //sprite.flip_modes = flip_modes;
    sprite.scale = scale;
    sprite.rotation = rotation;
    //sprite.color0 = color;
    //sprite.color1 = color;
    //sprite.color2 = color;
    //sprite.color3 = color;
    //sprite.texture = font->texture(0);

    // По идее все текстуры одинакового размера
    //vec2 pixel_size(1.f / sprite.texture->width(), 1.f / sprite.texture->height());

    vec2 char_pos = position;
    vec2 char_orig = origin;

    i32 i = 0;
    i32 step = 1;

    if (!!(flip_modes & FlipModes::horizontally))
    {
        i = (i32)unicode_text.size() - 1;
        step = -1;
    }

    Aabb string_aabb(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (; i >= 0 && i < (i32)unicode_text.size(); i += step)
    {
        auto it = font->glyphs().find(unicode_text[i]);

        if (it == font->glyphs().end())
            it = font->glyphs().find('?'); // Вопрос всегда должен быть в шрифте

        const Glyph& glyph = it->second;

        Rect rect(glyph.rect);
        vec2 offset(glyph.offset);

        //sprite.texture = font->texture(glyph.page);
        sprite.destination = Rect(char_pos, rect.size);
        //sprite.source_uv = Rect(rect.pos * pixel_size, rect.size * pixel_size);

        // Модифицируем origin, а не позицию, чтобы было правильное вращение
        sprite.origin = !!(flip_modes & FlipModes::vertically) ? char_orig - vec2(offset.x, font->line_height() - offset.y - rect.size.y) : char_orig - offset;

        Aabb char_aabb = measure_sprite_internal();
        string_aabb.merge(char_aabb);

        char_orig.x -= (f32)glyph.advance_x;
    }

    return string_aabb.to_rect();
}

} // namespace dviglo
