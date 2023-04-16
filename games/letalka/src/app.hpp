#pragma once

#include <dviglo/gl_utils/fbo.hpp>
#include <dviglo/main/application_base.hpp>

#include "global.hpp"


class App final : public ApplicationBase
{
    unique_ptr<Fbo> fbo_;
    unique_ptr<Global> global_;

public:
    App();

    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;

    void on_key(const SDL_KeyboardEvent& event_data);
};
