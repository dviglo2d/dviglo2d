#pragma once

#include "app_state_manager.hpp"

#include <dviglo/main/application_base.hpp>

using namespace dviglo;
using namespace std;


class App final : public ApplicationBase
{
    // Инициализируется в конструкторе
    inline static App* instance_ = nullptr;

    unique_ptr<AppStateManager> app_state_manager_;

public:
    static App* instance() { return instance_; }

    App();
    ~App() final;

    void on_frame_begin() final;
    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
    void on_mouse_button(const SDL_MouseButtonEvent& event_data);
};

#define APP (App::instance())
