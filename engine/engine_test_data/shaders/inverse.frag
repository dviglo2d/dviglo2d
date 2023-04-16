// Пример фильтра для Texture::apply_shader(...).
// Инвертирует изображение

#version 330 core

uniform sampler2D u_texture;
uniform int u_num_components;

in vec2 v_uv;

out vec4 f_color;

void main()
{
    vec4 color = texture(u_texture, v_uv);

    if (u_num_components >= 1)
        f_color.r = 1.0 - color.r;

    if (u_num_components >= 2)
        f_color.g = 1.0 - color.g;

    if (u_num_components >= 3)
        f_color.b = 1.0 - color.b;

    f_color.a = color.a;
}
