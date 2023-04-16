#pragma once

#include "puzzle_logic.hpp"

#include <dviglo/gl_utils/texture.hpp>
#include <dviglo/graphics/sprite_batch.hpp>

#include <SDL3_mixer/SDL_mixer.h>


// Обработка ввода и рендеринг игрового поля
class PuzzleInterface
{
private:
    Texture* spritesheet_;
    weak_ptr<PuzzleLogic> logic_;
    Mix_Chunk* tile_move_sound_ = nullptr;

public:
    PuzzleInterface(weak_ptr<PuzzleLogic> logic);
    ~PuzzleInterface();

    void on_click(vec2 mouse_pos);
    void draw(SpriteBatch* sprite_batch);
};
