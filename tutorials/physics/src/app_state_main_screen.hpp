#pragma once

#include "app_state_base.hpp"


class AppStateMainScreen : public AppStateBase
{
public:
    AppStateMainScreen()
    {
    }

    void on_enter() final
    {
    }

    void on_leave() final
    {
    }

    void on_key(const SDL_KeyboardEvent& event_data) final;
    void on_mouse_button(const SDL_MouseButtonEvent& event_data) final;

    void update(i64 ns) final;
    void draw() final;
};
