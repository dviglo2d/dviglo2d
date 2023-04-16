#pragma once

#include <dviglo/gl_utils/fbo.hpp>
#include <dviglo/main/application.hpp>

#include "global.hpp"


class App : public Application
{
    unique_ptr<Fbo> fbo_;
    unique_ptr<Global> global_;

public:
    App(const vector<StrUtf8>& args);

    void setup() override;
    void start() override;
    void handle_sdl_event(const SDL_Event& event) override;
    void update(u64 ns) override;
    void draw() override;

    void on_key(const SDL_KeyboardEvent& event_data);
};
