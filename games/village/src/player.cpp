#include "player.hpp"

#include <dviglo/gl_utils/texture_cache.hpp>

using namespace std;


void Player::draw(SpriteBatch& sprite_batch)
{
    fs::path base_path = get_base_path();
    shared_ptr<Texture> spritesheet = DV_TEXTURE_CACHE->get(base_path / "village_data/warrior_blue.png");
    Rect src(0.f, 0.f, 192.f, 192.f);
    sprite_batch.draw_sprite(spritesheet.get(), pos - src.size / 2.f, &src);
}
