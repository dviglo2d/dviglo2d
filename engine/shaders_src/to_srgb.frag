#version 450

layout(location = 0) in vec2 in_uv;
layout(set = 0, binding = 0) uniform sampler2D in_texture;
layout(location = 0) out vec4 out_color;

void main()
{
    vec4 linear_color = texture(in_texture, in_uv);

    // Неточное, но быстрое преобразование
    out_color = vec4(pow(linear_color.rgb, vec3(1.0 / 2.2)), linear_color.a);
}
