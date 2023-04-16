#pragma once

#include <dviglo/main/application.hpp>

#include "puzzle_interface.hpp"


class App : public Application
{
    Texture* spritesheet_ = nullptr;
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> my_font_;
    shared_ptr<PuzzleLogic> puzzle_logic_;
    shared_ptr<PuzzleInterface> puzzle_interface_;

public:
    App(const vector<StrUtf8>& args);

    void setup() override;
    void start() override;
    void handle_sdl_event(const SDL_Event& event) override;
    void update(u64 ns) override;
    void draw() override;

    void on_key(const SDL_KeyboardEvent& event_data);
    void on_mouse_button(const SDL_MouseButtonEvent& event_data);
};
