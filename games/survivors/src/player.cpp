#include "player.hpp"

#include <dv_math.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>
#include <SDL3/SDL.h>

using namespace std;


ivec2 Player::pos_to_tile(const Map& map, vec2 pos)
{
    const f32 tile_size_f = static_cast<f32>(Tile::size);
    ivec2 ret = static_cast<ivec2>(pos / tile_size_f);
    ref_clamp(ret.x, 0, map.size().x - 1);
    ref_clamp(ret.y, 0, map.size().y - 1);

    return ret;
}

void Player::update(i64 ns)
{
    f32 speed = 0.0000002f;

    i32 numkeys;
    const bool* keys = SDL_GetKeyboardState(&numkeys);

    vec2 new_pos = pos;

    if (keys[SDL_SCANCODE_W] && !keys[SDL_SCANCODE_S])
        new_pos.y -= ns * speed;
    else if (keys[SDL_SCANCODE_S] && !keys[SDL_SCANCODE_W])
        new_pos.y += ns * speed;

    if (keys[SDL_SCANCODE_A] && !keys[SDL_SCANCODE_D])
        new_pos.x -= ns * speed;
    else if (keys[SDL_SCANCODE_D] && !keys[SDL_SCANCODE_A])
        new_pos.x += ns * speed;

    pos = new_pos;
}
