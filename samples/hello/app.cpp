// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "app.h"

#include <dviglo/gl_utils/shader_cache.h>
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

    texture_ = make_unique<Texture>(base_path + "data/textures/tile128.png");
    sprite_batch_ = make_unique<SpriteBatch>();

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

    sprite_batch_->draw_sprite(texture_.get(), {100.f, 100.f});
    sprite_batch_->draw_sprite(texture_.get(), {500.f, 100.f}, nullptr, 0xFFFFFFFF, rotation);

    sprite_batch_->flush();
}
