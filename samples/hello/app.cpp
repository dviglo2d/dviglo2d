// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "app.h"

#include <dviglo/io/fs_base.h>

#include <glad/gl.h>

#include <iostream>


App::App(const std::vector<StrUtf8>& args)
    : Application(args)
{
    cout << "Командная строка: " << join(args, " ") << endl;

    log_path_ = "путь/к/логу";
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    cout << "Папка программы: " << base_path << endl;

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    float triangle_vertices[] = {
        -0.5f, -0.5f, 0.f,
         0.5f, -0.5f, 0.f,
         0.0f,  0.5f, 0.f
    };

    triangle_ = make_unique<VertexBuffer>(3, triangle_vertices, sizeof(triangle_vertices));
    
    float quad_vertices[] = {
        -0.9f, -0.9f, 0.f,
        -0.4f, -0.9f, 0.f,
        -0.4f, -0.4f, 0.f,

        -0.4f, -0.4f, 0.f,
        -0.9f, -0.4f, 0.f,
        -0.9f, -0.9f, 0.f
    };

    quad_ = make_unique<VertexBuffer>(6, quad_vertices, sizeof(quad_vertices));

    float quad2_vertices[] = {
        0.4f, 0.4f, 0.f, // Лево низ
        0.9f, 0.4f, 0.f, // Право низ
        0.9f, 0.9f, 0.f, // Право верх
        0.4f, 0.9f, 0.f, // Лево верх
    };

    u16 quad2_indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    quad2_vertices_ = make_unique<VertexBuffer>(4, quad2_vertices, sizeof(quad2_vertices));
    quad2_indices_ = make_unique<IndexBuffer>(6, quad2_indices, sizeof(quad2_indices));

    basic_shader_ = make_unique<ShaderProgram>(base_path + "data/shaders/basic.vert", base_path + "data/shaders/basic.frag");

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

    triangle_->bind();
    glDrawArrays(GL_TRIANGLES, 0, triangle_->num_vertices());

    quad_->bind();
    glDrawArrays(GL_TRIANGLES, 0, quad_->num_vertices());

    quad2_vertices_->bind();
    quad2_indices_->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}
