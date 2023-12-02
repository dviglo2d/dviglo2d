#version 330 core

// Чтобы не вычислять (2 / ширина_экрана, -2 / высота_экрана) для каждой вершины,
// вычисляется 1 раз на CPU
uniform vec2 u_scale;

// Атрибуты вершины
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

out vec4 v_color;

void main()
{
    // Преобразуем оконные координаты в NDC
    vec2 pos = a_pos * u_scale;
    pos += vec2(-1.0, 1.0);
    gl_Position = vec4(pos, 0.0, 1.0);

    v_color = a_color;
}
