#include "app.hpp"

#include <dviglo/main/engine_params.hpp>

using namespace glm;


App::App()
{
}

App::~App()
{
}

void App::setup()
{
    engine_params::window_size = ivec2(720, 700);
    engine_params::window_title = "Кликер";
    engine_params::vsync = -1;
    engine_params::window_mode = WindowMode::windowed;
}

void App::start()
{
    app_state_manager_ = make_unique<AppStateManager>();
    app_state_manager_->set_required_app_state_id(AppStateId::app_state_main_screen);
}

void App::on_frame_begin()
{
    app_state_manager_->apply();
}

void App::handle_sdl_event(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        app_state_manager_->get_current_app_state()->on_key(event.key);
        return;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        app_state_manager_->get_current_app_state()->on_mouse_button(event.button);
        return;

    default:
        // Реагируем на закрытие приложения и изменение размера окна
        ApplicationBase::handle_sdl_event(event);
        return;
    }
}

void App::on_key(const SDL_KeyboardEvent& event_data)
{
    app_state_manager_->get_current_app_state()->on_key(event_data);
}

void App::on_mouse_button(const SDL_MouseButtonEvent& event_data)
{
    app_state_manager_->get_current_app_state()->on_mouse_button(event_data);
}

void App::update(i64 ns)
{
    app_state_manager_->get_current_app_state()->update(ns);
}

void App::draw()
{
    app_state_manager_->get_current_app_state()->draw();
}
