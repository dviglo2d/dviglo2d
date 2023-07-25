// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "ecs/systems/s_test.hpp"

#include <dviglo/gl_utils/index_buffer.hpp>
#include <dviglo/gl_utils/shader_program.hpp>
#include <dviglo/gl_utils/texture.hpp>
#include <dviglo/gl_utils/vertex_buffer.hpp>
#include <dviglo/graphics/sprite_batch.hpp>
#include <dviglo/graphics/sprite_font.hpp>
#include <dviglo/main/application.hpp>

#include <SDL3/SDL_mixer.h>

using namespace std;


class App : public Application
{
    entt::registry ecs_;
    STest s_test_;

    ShaderProgram* textured_shader_;
    Texture* texture_;
    ShaderProgram* vert_color_shader_prog_;
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> font_;

    Mix_Music* music_ = nullptr;

public:
    App(const vector<StrUtf8>& args);
    ~App();

    entt::registry& ecs() { return ecs_; }

    void setup() override;
    void start() override;
    void update(u64 ns) override;
    void draw() override;
};
