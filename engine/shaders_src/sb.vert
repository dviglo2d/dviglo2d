#version 460

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in uint inTexIndex;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;
layout(location = 2) flat out uint fragTexIndex;

// Конвертация пикселей в координаты Vulkan (от -1.0 до 1.0)
// Для теста используем жестко зашитый размер экрана 512x512, как у вас в Viewport
vec2 normalizeScreenCoords(vec2 pos) {
    return vec2((pos.x / 512.0) * 2.0 - 1.0, (pos.y / 512.0) * 2.0 - 1.0);
}

void main() {
    gl_Position = vec4(normalizeScreenCoords(inPosition), 0.0, 1.0);
    fragUV = inUV;
    fragColor = inColor;
    fragTexIndex = inTexIndex;
}
