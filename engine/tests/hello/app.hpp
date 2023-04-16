#pragma once

#include <dviglo/audio/audio.hpp>
#include <dviglo/graphics/sprite_batch.hpp>
#include <dviglo/main/application_base.hpp>

using namespace dviglo;
using namespace std;


class App final : public ApplicationBase
{
private:
    shared_ptr<Texture> texture_;
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> font_;
    MIX_Audio* music_ = nullptr;
    MIX_Track* music_track_ = nullptr;

    // Отрендеренная в текстуру сцена
    unique_ptr<Texture> rendered_scene_;

public:
    App();
    ~App() final;

    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
};
