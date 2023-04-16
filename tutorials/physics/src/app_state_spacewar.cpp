#include "app_state_spacewar.hpp"

#include "app.hpp"

#include <imgui.h>


void AppStateSpacewar::on_key(const SDL_KeyboardEvent& event_data)
{
    if (event_data.type == SDL_EVENT_KEY_DOWN && event_data.repeat == false
        && event_data.scancode == SDL_SCANCODE_ESCAPE)
    {
        APP_STATE_MANAGER->required_app_state_id(AppStateId::main_screen);
    }
}

void AppStateSpacewar::on_mouse_button(const SDL_MouseButtonEvent& event_data)
{
    (void)event_data;
}

void AppStateSpacewar::update(i64 ns)
{
    (void)ns;

    //ImGui::ShowDemoWindow(nullptr);
}

void AppStateSpacewar::draw()
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    StrUtf8 title_str = "Spacewar (ESC - назад)";
    sprite_batch_->draw_string(title_str, r_20_font_.get(), vec2(10.f, 10.f));

    sprite_batch_->flush();
}
