#include "global.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/fs/log.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>


Global::Global()
{
    reg_ = make_unique<registry>();
    sprite_batch_ = make_unique<SpriteBatch>();

    StrUtf8 base_path = get_base_path();
    spritesheet_ = DV_TEXTURE_CACHE->get(base_path + "letalka_data/textures/spritesheet.png");
    r_20_font_ = make_unique<SpriteFont>(base_path + "engine_test_data/fonts/ubuntu-r_20_simple.fnt");

    instance_ = this;
    DV_LOG->write_debug("Global constructed");
}

Global::~Global()
{
    instance_ = nullptr;
    DV_LOG->write_debug("Global destructed");
}
