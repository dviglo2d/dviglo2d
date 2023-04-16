#pragma once

#include "map.hpp"

#include <dviglo/graphics/sprite_batch.hpp>
#include <glm/glm.hpp>

using namespace dviglo;
using namespace glm;


struct Player
{
    // В каком тайле находятся требуемые мировые координаты
    static ivec2 pos_to_tile(const Map& map, vec2 pos);

    // Мировая позиция ступней персонажа
    vec2 pos = vec2(200.f, 200.f);

    // Половина размера коллайдера.
    // Размер коллайдера (62x62) немного меньше размера тайла (64x64), чтобы легче попадать в проходы
    ivec2 half_size = ivec2(31, 31);

    void update(i64 ns);
};
