#pragma once

#include "app_state_base.hpp"

#include <dv_big_int.hpp>


class AppStateMainScreen : public AppStateBase
{
    // Количество золота
    BigInt gold_;

    // Количество золота, получаемого за клик
    BigInt power_{ 1 };

public:
    AppStateMainScreen()
        : AppStateBase()
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

    void draw() final;
};
