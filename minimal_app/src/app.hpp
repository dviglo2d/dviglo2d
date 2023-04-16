#pragma once

#include "my_subsystem.hpp"

#include <dviglo/graphics/sprite_batch.hpp>
#include <dviglo/main/application_base.hpp>

using namespace dviglo;
using namespace std;


class App final : public ApplicationBase
{
    // Инициализируется в конструкторе
    inline static App* instance_ = nullptr;

    unique_ptr<MySubsystem> my_subsystem_;
    shared_ptr<Texture> texture_;
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> r_20_font_;
    unique_ptr<SpriteFont> my_font_;

public:
    static App* instance() { return instance_; }

    App();
    ~App() final;

    void on_frame_begin() final {}
    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;
    void on_frame_end() final {}

    void on_key(const SDL_KeyboardEvent& event_data);
};

#define APP (App::instance())
