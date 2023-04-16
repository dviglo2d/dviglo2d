// Пример шейдера для Texture::apply_shader(...).
// Инвертирует изображение

#version 330 core

// Атрибуты вершины
layout (location = 0) in vec2 a_pos;

out vec2 v_uv;

void main()
{
    v_uv = a_pos * 0.5 + 0.5;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
