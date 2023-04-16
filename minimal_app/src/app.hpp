#pragma once

#include <dviglo/graphics/sprite_batch.hpp>
#include <dviglo/main/application.hpp>

using namespace dviglo;
using namespace std;


class App final : public Application
{
    shared_ptr<Texture> texture_;
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> r_20_font_;
    unique_ptr<SpriteFont> my_font_;

public:
    App(const vector<StrUtf8>& args);
    ~App() final;

    void setup() final;
    void start() final;
    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
};
