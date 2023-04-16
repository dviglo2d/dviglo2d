#pragma once

#include <dviglo/main/application_base.hpp>

#include "puzzle_interface.hpp"


class App final : public ApplicationBase
{
    shared_ptr<Texture> spritesheet_;
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> my_font_;
    shared_ptr<PuzzleLogic> puzzle_logic_;
    shared_ptr<PuzzleInterface> puzzle_interface_;

public:
    App();

    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
    void on_mouse_button(const SDL_MouseButtonEvent& event_data);
};
