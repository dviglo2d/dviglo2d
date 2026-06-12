#version 460

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;
layout(location = 2) flat in uint fragTexIndex;

layout(location = 0) out vec4 outColor;

void main() {
    // Пока мы не прикрутили bindless текстуры, мы просто выводим цвет вершин!
    outColor = fragColor;
}
