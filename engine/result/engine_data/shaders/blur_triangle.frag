// Размывает изображение по вертикали или по горизонтали (зависит от u_direction)

#version 330 core

uniform sampler2D u_texture;
uniform ivec2 u_size;
uniform vec2 u_inv_size; // Размер пикселя
uniform int u_num_components;
uniform int u_blur_radius;
uniform ivec2 u_direction;

in vec2 v_uv;

out vec4 f_color;

void main()
{
    f_color = texture(u_texture, v_uv) * (u_blur_radius + 1);

    for (int dist = 1; dist <= u_blur_radius; ++dist)
    {
        float weight = 1 + u_blur_radius - dist;
        vec2 offset = vec2(u_direction) * float(dist) * u_inv_size;
        f_color += texture(u_texture, v_uv + offset) * weight;
        f_color += texture(u_texture, v_uv - offset) * weight;
    }

    int total_weight = (u_blur_radius + 1) * (u_blur_radius + 1);
    f_color /= float(total_weight);
}
