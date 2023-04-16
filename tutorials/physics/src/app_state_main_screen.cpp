#include "app_state_main_screen.hpp"

#include "app.hpp"
#include "app_state_manager.hpp"

#include <imgui.h>


void AppStateMainScreen::on_key(const SDL_KeyboardEvent& event_data)
{
    if (event_data.type == SDL_EVENT_KEY_DOWN && event_data.repeat == false
        && event_data.scancode == SDL_SCANCODE_ESCAPE)
    {
        APP->exit();
    }
}

void AppStateMainScreen::on_mouse_button(const SDL_MouseButtonEvent& event_data)
{
    (void)event_data;

    // TODO
    APP_STATE_MANAGER->required_app_state_id(AppStateId::spacewar);
}

void AppStateMainScreen::update(i64 ns)
{
    (void)ns;

    ImGui::ShowDemoWindow(nullptr);
}

void AppStateMainScreen::draw()
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    StrUtf8 title_str = "Список примеров";
    sprite_batch_->draw_string(title_str, r_20_font_.get(), vec2(10.f, 10.f));

    sprite_batch_->flush();
}
