#pragma once

#include <dviglo/graphics/sprite_batch.hpp>
#include <SDL3/SDL.h>

using namespace dviglo;
using namespace glm;
using namespace std;


class AppStateBase
{
public:
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> r_20_font_;

    AppStateBase();
    virtual ~AppStateBase() = default;

    // Запрещаем копирование
    AppStateBase(const AppStateBase&) = delete;
    AppStateBase& operator =(const AppStateBase&) = delete;

    virtual void on_enter() {}
    virtual void on_leave() {}

    virtual void on_key(const SDL_KeyboardEvent& event_data) { (void)event_data; }
    virtual void on_mouse_button(const SDL_MouseButtonEvent& event_data) { (void)event_data; }

    virtual void update(i64 ns) { (void)ns; }
    virtual void draw() {}
};
