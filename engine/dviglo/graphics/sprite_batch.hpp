// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../gl_utils/index_buffer.hpp"
#include "../gl_utils/shader_program.hpp"
#include "../gl_utils/texture.hpp"
#include "../gl_utils/vertex_buffer.hpp"
#include "../math/rect.hpp"
#include "../res/sprite_font.hpp"

#include <memory>


namespace dviglo
{

// Режимы зеркального отображения спрайтов и текста
enum class FlipModes : u32
{
    none         = 0,
    horizontally = 1 << 0,
    vertically   = 1 << 1,
    both = horizontally | vertically
};
DV_FLAGS(FlipModes);


class SpriteBatch
{
    // ============================ Пакетный рендеринг треугольников ============================

private:

    // Максимальное число треугольников в порции
    inline static constexpr i32 max_triangles_in_portion_ = 512;

    // Число вершин в треугольнике
    inline static constexpr i32 vertices_per_triangle_ = 3;

    // Атрибуты вершин треугольников
    struct TVertex
    {
        glm::vec2 position;
        u32 color; // Цвет в формате 0xAABBGGRR
    };

    // Текущая порция треугольников
    TVertex t_vertices_[max_triangles_in_portion_ * vertices_per_triangle_];

    // Число вершин в массиве t_vertices_
    i32 t_num_vertices_ = 0;

    // Шейдерная программа для рендеринга треугольников
    ShaderProgram* t_shader_program_;

    // Вершинный буфер для треугольников (индексный буфер не используется)
    std::unique_ptr<VertexBuffer> t_vertex_buffer_;

public:

    // Данные для функции add_triangle().
    // Заполняем заранее выделенную память, вместо передачи кучи аргументов в функцию
    struct
    {
        TVertex v0, v1, v2;
    } triangle_;

    // Добавляет 3 вершины в массив t_vertices_. Вызывает flush(), если массив полон.
    // Перед вызовом этой функции необходимо заполнить структуру triangle_
    void add_triangle();

    // Указывает цвет для следующих треугольников (в формате 0xAABBGGRR)
    void set_shape_color(u32 color);

    // ============================ Пакетный рендеринг четырёхугольников ============================

private:

    // Максимальное число четырёхугольников в порции
    inline static constexpr i32 max_quads_in_portion_ = 500;

    // Четырёхугольник состоит из двух треугольников, а значит у него 6 вершин.
    // То есть каждый четырёхугольник занимает 6 элементов в индексном буфере
    inline static constexpr i32 indices_per_quad_ = 6;

    // Две вершины четырёхугольника идентичны для обоих треугольников, поэтому
    // в вершинном буфере каждый четырёхугольник занимает 4 элемента
    inline static constexpr u16 vertices_per_quad_ = 4;

    // Атрибуты вершин четырёхугольников
    struct QVertex
    {
        glm::vec2 position;
        u32 color; // Цвет в формате 0xAABBGGRR
        glm::vec2 uv;
    };

    // Текущая порция четырёхугольников
    QVertex q_vertices_[max_quads_in_portion_ * vertices_per_quad_];

    // Число вершин в массиве q_vertices_
    i32 q_num_vertices_ = 0;

    // Текущая текстура для четырёхугольников
    Texture* q_current_texture_ = nullptr;

    // Дефолтная шейдерная программа для четырёхугольников
    ShaderProgram* q_default_shader_program_;

    // Текущая шейдерная программа для четырёхугольников
    ShaderProgram* q_current_shader_program_;

    // Вершинный буфер для четырёхугольников
    std::unique_ptr<VertexBuffer> q_vertex_buffer_;

    // Индексный буфер для четырёхугольников
    std::unique_ptr<IndexBuffer> q_index_buffer_;

public:

    // Данные для функции add_quad().
    // Заполняем заранее выделенную память, вместо передачи кучи аргументов в функцию
    struct
    {
        Texture* texture;
        ShaderProgram* shader_program;
        QVertex v0, v1, v2, v3;
    } quad;

    // Добавляет 4 вершины в массив q_vertices_.
    // Если массив полон или требуемые шейдеры или текстура отличаются от текущих, то автоматически
    // происходит вызов функции flush() (то есть начинается новая порция).
    // Перед вызовом этой функции необходимо заполнить структуру quad
    void add_quad();

    // ============================ Общее ============================

private:

    // Вертикальное отражение необходимо при рендеринге в текстуру
    bool flip_vertically_ = false;

public:

    SpriteBatch();

    // Настраивает OpenGL для работы со SpriteBatch.
    // Для корректного рендеринга текста альфа-смешение должно быть включено.
    // Вертикальное отражение необходимо при рендеринге в текстуру
    void prepare_ogl(bool alpha_blending = true, bool flip_vertically = false);

    // Рендерит накопленную геометрию (то есть текущую порцию)
    void flush();

    // ======================= Используем пакетный рендеринг треугольников =======================

    void draw_triangle(glm::vec2 v0, glm::vec2 v1, glm::vec2 v2);

    void draw_rect(const Rect& rect);

    // Рисует круг
    void draw_disk(glm::vec2 center_pos, f32 radius, i32 num_segments);

    // ======================= Используем пакетный рендеринг четырёхугольников =======================

private:

    // Данные для функции draw_sprite()
    struct
    {
        Texture* texture;
        ShaderProgram* shader_program;
        Rect destination;
        Rect source_uv; // Текстурные координаты в диапазоне [0, 1]
        FlipModes flip_modes;
        glm::vec2 scale;
        f32 rotation;
        glm::vec2 origin;
        u32 color0; // Верхний левый угол
        u32 color1; // Верхний правый угол
        u32 color2; // Нижний правый угол
        u32 color3; // Нижний левый угол
    } sprite;

    // Берёт данные из sprite, вычисляет позиции вершин и записывает их в quad
    void transform_sprite_internal();

    // Перед вызовом этой функции нужно заполнить структуру sprite. Функция может изменить данные в структуре
    void draw_sprite_internal();

    // Перед вызовом этой функции нужно заполнить структуру sprite. Функция может изменить данные в структуре
    Aabb measure_sprite_internal();

public:

    // color - цвет в формате 0xAABBGGRR
    void draw_sprite(Texture* texture, const Rect& destination, const Rect* source = nullptr, u32 color = 0xFFFFFFFF,
        f32 rotation = 0.f, glm::vec2 origin = {0.f, 0.f}, glm::vec2 scale = {1.f, 1.f}, FlipModes flip_modes = FlipModes::none);

    Rect measure_sprite(const Rect& destination,
        f32 rotation = 0.f, glm::vec2 origin = {0.f, 0.f}, glm::vec2 scale = {1.f, 1.f});

    // color - цвет в формате 0xAABBGGRR
    void draw_sprite(Texture* texture, glm::vec2 position, const Rect* source = nullptr, u32 color = 0xFFFFFFFF,
        f32 rotation = 0.f, glm::vec2 origin = {0.f, 0.f}, glm::vec2 scale = {1.f, 1.f}, FlipModes flip_modes = FlipModes::none);

    Rect measure_sprite(Texture* texture, glm::vec2 position = {0.f, 0.f}, const Rect* source = nullptr,
        f32 rotation = 0.f, glm::vec2 origin = {0.f, 0.f}, glm::vec2 scale = {1.f, 1.f});

    // color - цвет в формате 0xAABBGGRR
    void draw_string(const StrUtf8& text, SpriteFont* font, glm::vec2 position, u32 color = 0xFFFFFFFF,
        f32 rotation = 0.0f, glm::vec2 origin = {0.f, 0.f}, glm::vec2 scale = {1.f, 1.f}, FlipModes flip_modes = FlipModes::none);

    Rect measure_string(const StrUtf8& text, SpriteFont* font, glm::vec2 position = {0.f, 0.f},
        f32 rotation = 0.0f, glm::vec2 origin = {0.f, 0.f}, glm::vec2 scale = {1.f, 1.f}, FlipModes flip_modes = FlipModes::none);
};

} // namespace dviglo
