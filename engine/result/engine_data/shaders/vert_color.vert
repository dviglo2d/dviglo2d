#version 330 core

// Вертикальное отражение необходимо при рендеринге в текстуру
uniform bool u_flip_vertically;

// Чтобы не вычислять (2 / ширина_экрана, 2 / высота_экрана) для каждой вершины, вычисляется 1 раз на CPU
uniform vec2 u_pixel_size;

// Атрибуты вершины
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

out vec4 v_color;

void main()
{
    // Преобразуем оконные координаты в NDC
    if (u_flip_vertically)
    {
        vec2 pos = a_pos * u_pixel_size;
        pos += vec2(-1.0, -1.0);
        gl_Position = vec4(pos, 0.0, 1.0);
    }
    else
    {
        vec2 pos = a_pos * vec2(u_pixel_size.x, -u_pixel_size.y);
        pos += vec2(-1.0, 1.0);
        gl_Position = vec4(pos, 0.0, 1.0);
    }

    v_color = a_color;
}
