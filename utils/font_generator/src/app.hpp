#pragma once

#include "preview_window.hpp"

#include <dv_imgui_application.hpp>

using namespace dviglo;
using namespace std;


class App final : public ImGuiApplication
{
    unique_ptr<SpriteFont> generated_font_;
    unique_ptr<PreviewWindow> preview_window_;

public:
    App();
    ~App() final;

    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
    void show_ui(i64 ns);
};
