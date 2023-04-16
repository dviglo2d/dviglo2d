#pragma once

#include <dviglo/graphics/sprite_batch.hpp>
#include <glm/glm.hpp>

using namespace dviglo;
using namespace glm;


struct Player
{
    // Позиция центра
    vec2 pos = vec2(200.f, 200.f);

    // Половина размера
    vec2 half_size = vec2(96, 96);

    void draw(SpriteBatch& sprite_batch);
};
