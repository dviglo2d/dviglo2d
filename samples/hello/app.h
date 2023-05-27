// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "ecs/systems/s_test.h"

#include <dviglo/gl_utils/index_buffer.h>
#include <dviglo/gl_utils/shader_program.h>
#include <dviglo/gl_utils/texture.h>
#include <dviglo/gl_utils/vertex_buffer.h>
#include <dviglo/graphics/sprite_batch.h>
#include <dviglo/main/application.h>

using namespace std;


class App : public Application
{
    entt::registry ecs_;
    STest s_test_;

    unique_ptr<ShaderProgram> basic_shader_;
    unique_ptr<ShaderProgram> textured_shader_;

    unique_ptr<VertexBuffer> triangle_;

    unique_ptr<VertexBuffer> quad_vertices_;
    unique_ptr<IndexBuffer> quad_indices_;

    unique_ptr<Texture> texture_;

    unique_ptr<VertexBuffer> textured_quad_vertices_;
    unique_ptr<IndexBuffer> textured_quad_indices_;

    unique_ptr<ShaderProgram> vert_color_shader_prog_;
    unique_ptr<SpriteBatch> sprite_batch_;

public:
    App(const vector<StrUtf8>& args);

    entt::registry& ecs() { return ecs_; }

    void start() override;
    void update(u64 ms) override;
    void draw() override;
};
