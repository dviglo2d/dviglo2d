#pragma once

#include <dv_imgui_application.hpp>

using namespace dviglo;
using namespace std;


class App : public ImGuiApplication
{
public:
    App(const vector<StrUtf8>& args);
    ~App() override;

    void setup() override;
    void start() override;

    void on_key(const SDL_KeyboardEvent& event_data);
    void handle_sdl_event(const SDL_Event& event) override;

    void show_ui(i64 ns);
    void update(i64 ns) override;

    void draw() override;
};
