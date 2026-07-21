// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../vulkan/vulkan_image.hpp"

//#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <stdexcept>

namespace dviglo
{
/*
class SpriteBatch
{
private:
    VulkanImage offscreen_image_;

    SpriteBatch() = default;

public:
    static std::optional<SpriteBatch> create(const vma::Allocator& vma_allocator, glm::uvec2 size);

    // Рендерит накопленную геометрию (то есть текущую порцию)
    void flush();
};

// Нельзя копировать
static_assert(!std::is_copy_constructible_v<SpriteBatch>);
static_assert(!std::is_copy_assignable_v<SpriteBatch>);

// Можно перемещать
static_assert(std::is_move_constructible_v<SpriteBatch>);
static_assert(std::is_move_assignable_v<SpriteBatch>);
*/

// Максимально плотная структура (24 байта). Идеально ложится в кэшлинии GPU.
struct BatchVertex {
    glm::vec2 position;
    glm::vec2 uv;
    uint32_t  color;        // RGBA (8 бит на канал)
    uint32_t  textureIndex; // Индекс в массиве bindless текстур (0 = белая текстура для геометрии)
};

class SpriteBatch {
public:
    // Размеры одного чанка памяти. 
    // Если кадр превысит эти значения, класс автоматически выделит новый чанк.
    static constexpr uint32_t MAX_VERTICES_PER_CHUNK = 65536; // 1.5 MB
    static constexpr uint32_t MAX_INDICES_PER_CHUNK  = 98304; // 384 KB

    // Максимальное количество уникальных текстур в одном кадре/пуле
    static constexpr uint32_t MAX_BINDLESS_TEXTURES  = 1024; 

    // В конструктор передаем девайс, аллокатор и белую текстуру 1x1 для инициализации нулевого индекса.
    SpriteBatch(vk::Device device, vma::Allocator allocator, vk::ImageView whiteTexture) 
        : m_device(device), m_allocator(allocator) 
    {
        InitBindlessSystem();
        
        // Индекс 0 всегда резервируем под белую текстуру (для отрисовки геометрии)
        GetTextureIndex(whiteTexture); 

        AllocateChunk(); // Выделяем первый чанк при запуске
    }

    ~SpriteBatch() {
        for (auto& chunk : m_chunks) {
            m_allocator.destroyBuffer(chunk.vertexBuffer, chunk.vertexAlloc);
            m_allocator.destroyBuffer(chunk.indexBuffer, chunk.indexAlloc); // Используем поле indexBuffer из структуры
        }

        m_device.destroySampler(m_sampler);
        m_device.destroyDescriptorSetLayout(m_bindlessLayout);
        m_device.destroyDescriptorPool(m_descriptorPool);
    }

    // Запрет копирования
    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    // ------------------------------------------------------------------------
    // РЕГИСТРАЦИЯ ТЕКСТУР
    // ------------------------------------------------------------------------
    // Передаешь сюда свой ImageView, получаешь индекс для записи в BatchVertex
    uint32_t GetTextureIndex(vk::ImageView imageView) 
    {
        // Проверяем кэш: если текстура уже в массиве, просто возвращаем ее индекс
        if (auto it = m_textureCache.find(imageView); it != m_textureCache.end()) {
            return it->second;
        }

        if (m_currentTextureIndex >= MAX_BINDLESS_TEXTURES) {
            throw std::runtime_error("SpriteBatch: Exceeded MAX_BINDLESS_TEXTURES limit!");
        }

        uint32_t index = m_currentTextureIndex++;

        // Обновляем дескриптор на видеокарте
        vk::DescriptorImageInfo imageInfo {
            .imageView   = imageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        };

        vk::WriteDescriptorSet writeDesc {
            .dstSet          = m_bindlessSet,
            .dstBinding      = 1,
            .dstArrayElement = index, // Записываем именно по этому индексу!
            .descriptorCount = 1,
            .descriptorType  = vk::DescriptorType::eSampledImage,
            .pImageInfo      = &imageInfo,
        };

        m_device.updateDescriptorSets(1, &writeDesc, 0, nullptr);

        // Сохраняем в кэш
        m_textureCache[imageView] = index;
        return index;
    }

    void Begin(vk::CommandBuffer cmdBuffer, vk::PipelineLayout pipelineLayout) {
        m_cmd = cmdBuffer;
        m_currentChunkIndex = 0;
        m_vertexCount = 0;
        m_indexCount = 0;

        // Переводим горячие указатели на начало первого чанка
        m_mappedVertices = m_chunks[0].mappedVertices;
        m_mappedIndices  = m_chunks[0].mappedIndices;

        // Биндим наш огромный массив текстур на 0 слот один раз за весь Begin!
        m_cmd.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, 
            pipelineLayout, 
            0, // первый set
            1, &m_bindlessSet, 
            0, nullptr
        );
    }

    // ------------------------------------------------------------------------
    // РИСОВАНИЕ СПРАЙТОВ (Квадратов)
    // ------------------------------------------------------------------------
    inline void DrawSprite(const glm::vec2& pos, const glm::vec2& size, 
                           const glm::vec4& uvRect, uint32_t color, uint32_t texIndex) 
    {
        // Если в текущем чанке не хватает места для 4 вершин и 6 индексов -> сбрасываем (Flush)
        if (m_vertexCount + 4 > MAX_VERTICES_PER_CHUNK || m_indexCount + 6 > MAX_INDICES_PER_CHUNK) {
            Flush();
        }

        uint32_t vIdx = m_vertexCount;

        // Прямая запись (Zero-Copy) в Write-Combined память VRAM/PCIe
        m_mappedVertices[vIdx + 0] = { pos, {uvRect.x, uvRect.y}, color, texIndex };
        m_mappedVertices[vIdx + 1] = { {pos.x + size.x, pos.y}, {uvRect.x + uvRect.z, uvRect.y}, color, texIndex };
        m_mappedVertices[vIdx + 2] = { {pos.x + size.x, pos.y + size.y}, {uvRect.x + uvRect.z, uvRect.y + uvRect.w}, color, texIndex };
        m_mappedVertices[vIdx + 3] = { {pos.x, pos.y + size.y}, {uvRect.x, uvRect.y + uvRect.w}, color, texIndex };

        uint32_t iIdx = m_indexCount;
        m_mappedIndices[iIdx + 0] = vIdx + 0;
        m_mappedIndices[iIdx + 1] = vIdx + 1;
        m_mappedIndices[iIdx + 2] = vIdx + 2;
        m_mappedIndices[iIdx + 3] = vIdx + 2;
        m_mappedIndices[iIdx + 4] = vIdx + 3;
        m_mappedIndices[iIdx + 5] = vIdx + 0;

        m_vertexCount += 4;
        m_indexCount += 6;
    }

    // ------------------------------------------------------------------------
    // РИСОВАНИЕ КАСТОМНОЙ ГЕОМЕТРИИ (Треугольников, используется для кругов/линий)
    // ------------------------------------------------------------------------
    // Для заливки цветом передавайте texIndex = 0 (и убедитесь, что под индексом 0 у вас белая 1x1 текстура)
    inline void DrawTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, 
                             uint32_t color, uint32_t texIndex = 0) 
    {
        if (m_vertexCount + 3 > MAX_VERTICES_PER_CHUNK || m_indexCount + 3 > MAX_INDICES_PER_CHUNK) {
            Flush();
        }

        uint32_t vIdx = m_vertexCount;

        m_mappedVertices[vIdx + 0] = { p1, {0.0f, 0.0f}, color, texIndex };
        m_mappedVertices[vIdx + 1] = { p2, {0.0f, 0.0f}, color, texIndex };
        m_mappedVertices[vIdx + 2] = { p3, {0.0f, 0.0f}, color, texIndex };

        uint32_t iIdx = m_indexCount;
        m_mappedIndices[iIdx + 0] = vIdx + 0;
        m_mappedIndices[iIdx + 1] = vIdx + 1;
        m_mappedIndices[iIdx + 2] = vIdx + 2;

        m_vertexCount += 3;
        m_indexCount += 3;
    }

    void End() {
        if (m_indexCount > 0) {
            Flush();
        }
        m_cmd = nullptr;
    }

    // Возвращает DescriptorSetLayout (понадобится при создании VkPipelineLayout)
    vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_bindlessLayout; }

private:
    struct Chunk {
        vk::Buffer vertexBuffer;
        vma::Allocation vertexAlloc;
        BatchVertex* mappedVertices;

        vk::Buffer indexBuffer;
        vma::Allocation indexAlloc;
        uint32_t* mappedIndices;
    };

    vk::Device m_device;
    vma::Allocator m_allocator;
    vk::CommandBuffer m_cmd;

    std::vector<Chunk> m_chunks;
    uint32_t m_currentChunkIndex = 0;

    // Горячие данные состояния (для максимальной скорости)
    uint32_t m_vertexCount = 0;
    uint32_t m_indexCount = 0;
    BatchVertex* m_mappedVertices = nullptr;
    uint32_t*    m_mappedIndices  = nullptr;

    // --- BINDLESS DATA ---
    vk::Sampler m_sampler;
    vk::DescriptorSetLayout m_bindlessLayout;
    vk::DescriptorPool m_descriptorPool;
    vk::DescriptorSet m_bindlessSet;
    
    uint32_t m_currentTextureIndex = 0;


    // Структура, которая объясняет C++, как хэшировать vk::ImageView
    struct ImageViewHasher {
        std::size_t operator()(vk::ImageView iv) const {
            // Приводим C++ обертку к голому C-типу (VkImageView) и хэшируем его
            return std::hash<VkImageView>()(static_cast<VkImageView>(iv));
        }
    };

    // Передаем наш кастомный хэшер третьим параметром шаблона
    std::unordered_map<vk::ImageView, uint32_t, ImageViewHasher> m_textureCache;


    void InitBindlessSystem() {
        // 1. Создаем Sampler для 2D-спрайтов
        vk::SamplerCreateInfo samplerInfo {
            .magFilter    = vk::Filter::eLinear, 
            .minFilter    = vk::Filter::eLinear,
            .mipmapMode   = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
        };
        m_sampler = m_device.createSampler(samplerInfo).value;

        // Binding 0: Единственный сэмплер
        vk::DescriptorSetLayoutBinding samplerBinding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eSampler, // !!!!!!!! -- Только сэмплер
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment,
        };

        // Binding 1: Массив раздельных текстур (без сэмплера)
        vk::DescriptorSetLayoutBinding texturesBinding{
            .binding = 1,
            .descriptorType = vk::DescriptorType::eSampledImage, // !!!!!!!! -- Только картинки
            .descriptorCount = MAX_BINDLESS_TEXTURES,
            .stageFlags = vk::ShaderStageFlagBits::eFragment,
        };

        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { samplerBinding, texturesBinding };

        // Флаги Bindless применяются ТОЛЬКО к массиву текстур (binding 1)
        std::array<vk::DescriptorBindingFlags, 2> bindingFlags = {
            vk::DescriptorBindingFlags{}, // Для сэмплера (binding 0) флаги не нужны
            vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind
        };

        vk::DescriptorSetLayoutBindingFlagsCreateInfo layoutFlagsInfo{
           .bindingCount = static_cast<uint32_t>(bindingFlags.size()), // !!!!!!!! -- 
           .pBindingFlags = bindingFlags.data(),                        // !!!!!!!! -- 
        };

        vk::DescriptorSetLayoutCreateInfo layoutInfo{
            .pNext = &layoutFlagsInfo,
            .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
            .bindingCount = static_cast<uint32_t>(bindings.size()), // !!!!!!!! --
            .pBindings = bindings.data(),                        // !!!!!!!! --
        };
        m_bindlessLayout = m_device.createDescriptorSetLayout(layoutInfo).value;

        // 3. Создаем Pool для ОБЕИХ сущностей
        // !!!!!!!! --
        std::array<vk::DescriptorPoolSize, 2> poolSizes = { {
            { vk::DescriptorType::eSampler, 1 },
            { vk::DescriptorType::eSampledImage, MAX_BINDLESS_TEXTURES }
        } };
        // !!!!!!!! --

        vk::DescriptorPoolCreateInfo poolInfo{
            .flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
            .maxSets = 1,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()), // !!!!!!!! --
            .pPoolSizes = poolSizes.data(),                        // !!!!!!!! --
        };
        m_descriptorPool = m_device.createDescriptorPool(poolInfo).value;

        // 4. Выделяем DescriptorSet (Здесь без изменений)
        vk::DescriptorSetAllocateInfo allocInfo{
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &m_bindlessLayout,
        };
        m_bindlessSet = m_device.allocateDescriptorSets(allocInfo).value.front();

        // 5. Единожды записываем Сэмплер в binding 0 (НОВАЯ ОПЕРАЦИЯ)
        // !!!!!!!! -- ЭТОГО БЛОКА РАНЬШЕ НЕ БЫЛО ВООВЩЕ -- !!!!!!!!
        vk::DescriptorImageInfo samplerDescInfo{
            .sampler = m_sampler,
        };
        vk::WriteDescriptorSet writeSampler{
            .dstSet = m_bindlessSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eSampler,
            .pImageInfo = &samplerDescInfo,
        };
        m_device.updateDescriptorSets(1, &writeSampler, 0, nullptr);

    }

    void AllocateChunk() {
        Chunk newChunk{};

        // 1. Создаем Vertex Buffer
        vk::BufferCreateInfo vtxInfo{
            .size = MAX_VERTICES_PER_CHUNK * sizeof(BatchVertex),
            .usage = vk::BufferUsageFlagBits::eVertexBuffer
        };

        // Флаг eHostAccessSequentialWrite - ГАРАНТИРУЕТ выделение Write-Combined памяти.
        // Запись сюда обходит кэш процессора, скорость передачи на видеокарту возрастает кратно.
        vma::AllocationCreateInfo allocInfo{
            .flags = vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
            .usage = vma::MemoryUsage::eAuto
        };

        vma::AllocationInfo vtxAllocInfo;
        auto vtxResult = m_allocator.createBuffer(vtxInfo, allocInfo, &vtxAllocInfo);
        newChunk.vertexBuffer = vtxResult.value.second;
        newChunk.vertexAlloc = vtxResult.value.first;
        newChunk.mappedVertices = static_cast<BatchVertex*>(vtxAllocInfo.pMappedData);

        // 2. Создаем Index Buffer
        vk::BufferCreateInfo idxInfo{
            .size = MAX_INDICES_PER_CHUNK * sizeof(uint32_t),
            .usage = vk::BufferUsageFlagBits::eIndexBuffer
        };

        vma::AllocationInfo idxAllocInfo;
        auto idxResult = m_allocator.createBuffer(idxInfo, allocInfo, &idxAllocInfo);
        newChunk.indexBuffer = idxResult.value.second;
        newChunk.indexAlloc = idxResult.value.first;
        newChunk.mappedIndices = static_cast<uint32_t*>(idxAllocInfo.pMappedData);

        m_chunks.push_back(newChunk);
    }

    void Flush() {
        if (m_indexCount == 0) return;

        Chunk& chunk = m_chunks[m_currentChunkIndex];

        // Биндим буферы для текущего чанка
        vk::DeviceSize offset = 0;
        m_cmd.bindVertexBuffers(0, 1, &chunk.vertexBuffer, &offset);
        m_cmd.bindIndexBuffer(chunk.indexBuffer, 0, vk::IndexType::eUint32);

        // Один Draw Call на весь чанк (спрайты и треугольники вместе)
        m_cmd.drawIndexed(m_indexCount, 1, 0, 0, 0);

        // Переходим к следующему чанку
        m_currentChunkIndex++;
        m_vertexCount = 0;
        m_indexCount = 0;

        // Если текущие чанки закончились — динамически создаем новый (защита от креша)
        if (m_currentChunkIndex >= m_chunks.size()) {
            AllocateChunk();
        }

        // Обновляем горячие указатели на новую память
        m_mappedVertices = m_chunks[m_currentChunkIndex].mappedVertices;
        m_mappedIndices  = m_chunks[m_currentChunkIndex].mappedIndices;
    }
};

} // namespace dviglo
