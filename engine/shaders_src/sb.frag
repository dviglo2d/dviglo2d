#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;
layout(location = 2) flat in uint fragTexIndex;

// Объявляем массив текстур (bindless)
// set и binding укажи те, которые используешь в своем VkDescriptorSetLayout
//layout(set = 0, binding = 0) uniform sampler2D textures[];

// 0 - Наш общий сэмплер
layout(binding = 0) uniform sampler spriteSampler; 

// 1 - Массив самих текстур (без сэмплера)
layout(binding = 1) uniform texture2D textures[]; 

layout(location = 0) out vec4 outColor;

void main() {
    // Получаем цвет из текстуры. 
    // nonuniformEXT ОБЯЗАТЕЛЕН, так как индекс может быть разным для соседних пикселей
    vec4 texColor = texture(sampler2D(textures[nonuniformEXT(fragTexIndex)], spriteSampler), fragUV);
    
    // Смешиваем цвет вершины и цвет текстуры
    outColor = fragColor * texColor;
}
