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

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    basic_shader_ = make_unique<ShaderProgram>(base_path + "data/shaders/basic.vert", base_path + "data/shaders/basic.frag");
    basic_shader_->use();

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
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
