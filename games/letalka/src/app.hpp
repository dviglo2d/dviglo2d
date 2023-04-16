#pragma once

#include <dviglo/gl_utils/fbo.hpp>
#include <dviglo/main/application.hpp>

#include "global.hpp"


class App final : public Application
{
    unique_ptr<Fbo> fbo_;
    unique_ptr<Global> global_;

public:
    App(const vector<StrUtf8>& args);

    void setup() final;
    void start() final;
    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
};
