#pragma once

#include <dv_imgui_application.hpp>

using namespace dviglo;
using namespace std;


class App final : public ImGuiApplication
{
public:
    App();
    ~App() final;

    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
    void show_ui(i64 ns);
};
