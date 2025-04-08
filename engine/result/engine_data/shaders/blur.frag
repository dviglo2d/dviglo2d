#version 330 core

uniform sampler2D u_texture;

in vec2 v_uv;

out vec4 f_color;

void main()
{
        float value = texture(u_texture, v_uv).r;
        // Пример обработки: инверсия
        f_color = vec4(vec3(1.0 - value), 1.0);
}
