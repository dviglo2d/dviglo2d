// Copyright (c) the Dviglo project
// License: MIT

#include "app.hpp"

#include <dviglo/gl_utils/shader_cache.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/io/fs_base.hpp>
#include <dviglo/main/engine_params.hpp>
#include <dviglo/main/os_window.hpp>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <format>
#include <iostream>

using namespace glm;
using namespace std;


App::App(const std::vector<StrUtf8>& args)
    : Application(args)
{
    cout << "Командная строка: " << join(args, " ") << endl;

    log_path_ = get_pref_path("dviglo", "samples") + "hello.log";
}

struct Vertex
{
    vec2 pos;
    u32 color;
    vec2 uv;
};

void App::setup()
{
    engine_params::window_size = ivec2(800, 800);
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    cout << "Папка программы: " << base_path << endl;

    texture_ = DV_TEXTURE_CACHE->get(base_path + "data/textures/tile128.png");
    sprite_batch_ = make_unique<SpriteBatch>();
    font_ = make_unique<SpriteFont>(base_path + "data/fonts/ubuntu-r_20_simple.fnt");

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

    StrUtf8 music_path = base_path + "data/audio/in_the_field.mp3";
    music_ = Mix_LoadMUS(music_path.c_str());

    if (!music_)
        DV_LOG->write_error(format("App::start(): !music_ | {}", SDL_GetError()));

    if (Mix_PlayMusic(music_, -1) < 0)
        DV_LOG->write_error(format("App::start(): Mix_PlayMusic(...) < 0 | {}", SDL_GetError()));
}

static float rotation = 0.f;

static StrUtf8 fps_text;

void App::update(u64 ns)
{
    rotation += ns * 0.000'000'000'1f;
    while (rotation >= 360.f)
        rotation -= 360.f;

    u64 fps = SDL_NS_PER_SECOND / ns;
    fps_text = format("FPS: {}", fps);

    s_test_.update(ecs_, ns);
}

void App::draw()
{
    ivec2 screen_size = DV_OS_WINDOW->size_in_pixels();

    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Задаём треугольники по часовой стрелке
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    sprite_batch_->triangle_.v0 = {{800.f, 0.f}, 0xFF00FF00};
    sprite_batch_->triangle_.v1 = {{800.f, 300.f}, 0xFF0000FF};
    sprite_batch_->triangle_.v2 = {{0.f, 300.f}, 0xFFFFFFFF};
    sprite_batch_->add_triangle();

    sprite_batch_->set_shape_color(0xFFFF0000);
    sprite_batch_->draw_triangle({400.f, 0.f}, {400.f, 600.f}, {0.f, 600.f});

    sprite_batch_->draw_sprite(texture_, {100.f, 100.f});
    sprite_batch_->draw_sprite(texture_, {500.f, 100.f}, nullptr, 0xFFFFFFFF, rotation);

    sprite_batch_->draw_string(fps_text, font_.get(), vec2{4.f, 1.f}, 0xFF000000);
    sprite_batch_->draw_string(fps_text, font_.get(), vec2{3.f, 0.f}, 0xFFFFFFFF);

    sprite_batch_->flush();
}

App::~App()
{
    Mix_FreeMusic(music_); // Проверка на nullptr не нужна
    music_ = nullptr;
}
