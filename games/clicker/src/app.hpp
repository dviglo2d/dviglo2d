#pragma once

#include <dv_big_int.hpp>
#include <dviglo/graphics/sprite_batch.hpp>
#include <dviglo/main/application.hpp>

using namespace dviglo;
using namespace std;


class App final : public Application
{
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> r_20_font_;

    // Количество золота
    BigInt gold_;

    // Количество золота, получаемого за клик
    BigInt power_{1};

public:
    App(const vector<StrUtf8>& args);
    ~App() final;

    void setup() final;
    void start() final;
    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
    void on_mouse_button(const SDL_MouseButtonEvent& event_data);
};
