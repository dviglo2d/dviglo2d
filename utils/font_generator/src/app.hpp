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
    App(const vector<StrUtf8>& args);
    ~App() final;

    void setup() final;
    void start() final;

    void on_key(const SDL_KeyboardEvent& event_data);
    void handle_sdl_event(const SDL_Event& event) final;

    void show_ui(i64 ns);
    void update(i64 ns) final;

    void draw() final;
};
