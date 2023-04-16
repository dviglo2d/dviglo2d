#pragma once

#include "puzzle_logic.hpp"

#include <dviglo/audio/audio.hpp>
#include <dviglo/gl_utils/texture.hpp>
#include <dviglo/graphics/sprite_batch.hpp>

#include <SDL3_mixer/SDL_mixer.h>


// Обработка ввода и рендеринг игрового поля
class PuzzleInterface
{
private:
    shared_ptr<Texture> spritesheet_;
    weak_ptr<PuzzleLogic> logic_;
    MIX_Audio* tile_move_sound_ = nullptr;
    MIX_Track* tile_move_track_ = nullptr;

public:
    PuzzleInterface(weak_ptr<PuzzleLogic> logic);
    ~PuzzleInterface();

    void on_click(vec2 mouse_pos);
    void draw(SpriteBatch* sprite_batch);
};
