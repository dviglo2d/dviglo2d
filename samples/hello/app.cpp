// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "app.h"

#include <dviglo/io/fs_base.h>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <iostream>

using namespace glm;


App::App(const std::vector<StrUtf8>& args)
    : Application(args)
{
    cout << "Командная строка: " << join(args, " ") << endl;

    log_path_ = "путь/к/логу";
}

struct Vertex
{
    vec2 pos;
    u32 color;
    vec2 uv;
};

void App::start()
{
    StrUtf8 base_path = get_base_path();
    cout << "Папка программы: " << base_path << endl;

    vec2 triangle_vertices[] = {
        {-0.5f, -0.5f},
        { 0.5f, -0.5f},
        { 0.0f,  0.5f},
    };

    triangle_ = make_unique<VertexBuffer>(3, VertexAttributes::position, BufferUsage::static_draw, triangle_vertices);

    float quad_vertices[] = {
        0.4f, 0.4f, // Лево низ
        0.9f, 0.4f, // Право низ
        0.9f, 0.9f, // Право верх
        0.4f, 0.9f, // Лево верх
    };

    u16 quad_indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    quad_vertices_ = make_unique<VertexBuffer>(4, VertexAttributes::position, BufferUsage::static_draw, quad_vertices);
    quad_indices_ = make_unique<IndexBuffer>(6, IndexType::u16, BufferUsage::static_draw, quad_indices);

    basic_shader_ = make_unique<ShaderProgram>(base_path + "data/shaders/basic.vert", base_path + "data/shaders/basic.frag");

    Vertex textured_quad_vertices[] = {
        {{-0.9f, 0.4f}, 0xFF0000FF, {0.f, 1.f}}, // Лево низ
        {{-0.4f, 0.4f}, 0xFFFFFFFF, {1.f, 1.f}}, // Право низ
        {{-0.4f, 0.9f}, 0xFFFF0000, {1.f, 0.f}}, // Право верх
        {{-0.9f, 0.9f}, 0xFF00FF00, {0.f, 0.f}}, // Лево верх
    };

    u32 textured_quad_indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    textured_quad_vertices_ = make_unique<VertexBuffer>(4,
        VertexAttributes::position | VertexAttributes::color | VertexAttributes::uv,
        BufferUsage::static_draw, textured_quad_vertices);

    textured_quad_indices_ = make_unique<IndexBuffer>(6, IndexType::u32, BufferUsage::static_draw, textured_quad_indices);
    textured_shader_ = make_unique<ShaderProgram>(base_path + "data/shaders/textured.vert", base_path + "data/shaders/textured.frag");
    texture_ = make_unique<Texture>(base_path + "data/textures/tile128.png");

    entt::entity ent1 = ecs_.create();

    CName& ent1_name = ecs_.emplace<CName>(ent1);
    ent1_name.value = "entity 1";

    CCounter& ent1_counter = ecs_.emplace<CCounter>(ent1);
    ent1_counter.value = 10;

    entt::entity ent2 = ecs_.create();

    CName& ent2_name = ecs_.emplace<CName>(ent2);
    ent2_name.value = "entity 2";

    CCounter& ent2_counter = ecs_.emplace<CCounter>(ent2);
    ent2_counter.value = 100;
}

void App::update(u64 ms)
{
    s_test_.update(ecs_, ms);
}

void App::draw()
{
    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    basic_shader_->use();

    basic_shader_->set("u_color", vec4(1.f, 0.f, 0.f, 1.f));
    triangle_->bind();
    glDrawArrays(GL_TRIANGLES, 0, triangle_->num_vertices());

    basic_shader_->set("u_color", vec4(0.f, 1.f, 1.f, 1.f));
    quad_vertices_->bind();
    quad_indices_->bind();
    glDrawElements(GL_TRIANGLES, quad_indices_->num_indices(), quad_indices_->type(), nullptr);

    textured_shader_->use();
    glActiveTexture(GL_TEXTURE0);
    texture_->bind();
    textured_quad_vertices_->bind();
    textured_quad_indices_->bind();
    glDrawElements(GL_TRIANGLES, textured_quad_indices_->num_indices(), textured_quad_indices_->type(), nullptr);
}
