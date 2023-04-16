#pragma once

#include <dv_imgui_application.hpp>

using namespace dviglo;
using namespace std;


class App final : public ImGuiApplication
{
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
