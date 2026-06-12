#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;
layout(location = 2) flat in uint fragTexIndex;

// Объявляем массив текстур (bindless)
// set и binding укажи те, которые используешь в своем VkDescriptorSetLayout
layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 0) out vec4 outColor;

void main() {
    // Получаем цвет из текстуры. 
    // nonuniformEXT ОБЯЗАТЕЛЕН, так как индекс может быть разным для соседних пикселей
    vec4 texColor = texture(textures[nonuniformEXT(fragTexIndex)], fragUV);
    
    // Смешиваем цвет вершины и цвет текстуры
    outColor = fragColor * texColor;
}
