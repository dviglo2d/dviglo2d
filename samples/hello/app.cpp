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

    textured_shader_ = make_unique<ShaderProgram>(base_path + "data/shaders/vert_color_texture.vert", base_path + "data/shaders/vert_color_texture.frag");
    texture_ = make_unique<Texture>(base_path + "data/textures/tile128.png");
    vert_color_shader_prog_ = make_unique<ShaderProgram>(base_path + "data/shaders/vert_color.vert", base_path + "data/shaders/vert_color.frag");
    sprite_batch_ = make_unique<SpriteBatch>(vert_color_shader_prog_.get());

    /*
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
    */
}

static float rotation = 0.f;

void App::update(u64 ms)
{
    rotation += ms * 0.0001f;
    while (rotation >= 360.f)
        rotation -= 360.f;

    s_test_.update(ecs_, ms);
}

ivec2 screen_size{800, 600};

void App::draw()
{
    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->triangle_.v0 = {{0.f, 300.f}, 0xFFFFFFFF};
    sprite_batch_->triangle_.v1 = {{800.f, 300.f}, 0xFF0000FF};
    sprite_batch_->triangle_.v2 = {{800.f, 0.f}, 0xFF00FF00};
    sprite_batch_->add_triangle();

    sprite_batch_->set_shape_color(0xFFFF0000);
    sprite_batch_->draw_triangle({0.f, 600.f}, {400.f, 600.f}, {400.f, 0.f});

    sprite_batch_->quad.texture = texture_.get();
    sprite_batch_->quad.shader_program = textured_shader_.get();
    sprite_batch_->quad.v0 = {{100.f, 300.f}, 0xFFFFFFFF, {0.f, 1.f}}; // Лево низ
    sprite_batch_->quad.v1 = {{400.f, 300.f}, 0xFFFFFFFF, {1.f, 1.f}}; // Право низ
    sprite_batch_->quad.v2 = {{400.f, 100.f}, 0xFFFFFFFF, {1.f, 0.f}}; // Право верх
    sprite_batch_->quad.v3 = {{100.f, 100.f}, 0xFFFFFFFF, {0.f, 0.f}}; // Лево верх
    sprite_batch_->add_quad();

    sprite_batch_->sprite.texture = texture_.get();
    sprite_batch_->sprite.shader_program = textured_shader_.get();
    sprite_batch_->sprite.destination = {{500.f, 100.f}, {600.f, 200.f}};
    sprite_batch_->sprite.source_uv = {{0.f, 0.f}, {1.f, 1.f}};
    sprite_batch_->sprite.flip_modes = FlipModes::none;
    sprite_batch_->sprite.scale = {1.f, 1.f};
    sprite_batch_->sprite.rotation = rotation;
    sprite_batch_->sprite.origin = {50.f, 50.f};
    sprite_batch_->sprite.color0 = 0xFFFFFFFF;
    sprite_batch_->sprite.color1 = 0xFFFFFFFF;
    sprite_batch_->sprite.color2 = 0xFFFFFFFF;
    sprite_batch_->sprite.color3 = 0xFFFFFFFF;
    sprite_batch_->draw_sprite();

    sprite_batch_->flush();
}
