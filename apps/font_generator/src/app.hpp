#pragma once

#include "preview_window.hpp"

#include <dviglo/main/application.hpp>

using namespace dviglo;
using namespace std;


class App : public Application
{
    unique_ptr<SpriteFont> generated_font_;
    unique_ptr<PreviewWindow> preview_window_;

public:
    App(const vector<StrUtf8>& args);
    ~App() override;

    void setup() override;
    void start() override;
    void handle_sdl_event(const SDL_Event& event) override;
    void update(u64 ns) override;
    void draw() override;

    void on_key(const SDL_KeyboardEvent& event_data);

    void show_ui();
};
