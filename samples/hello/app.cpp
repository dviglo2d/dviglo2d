// Copyright (c) the Dviglo project
// License: MIT

#include "app.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/main/engine_params.hpp>
#include <dviglo/main/os_window.hpp>

#include <format>
#include <iostream>

using namespace glm;


App::App(const vector<StrUtf8>& args)
    : Application(args)
{
}

void App::setup()
{
    engine_params::log_path = get_pref_path("dviglo2d", "samples") + "hello.log";
    engine_params::window_size = ivec2(800, 800);
    engine_params::window_resizable = true;
    engine_params::msaa_samples = 4; // При значении 8 крэшится на сервере ГитХаба в Линуксе
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    DV_LOG->write_info(format("Командная строка: {}", join(args(), " ")));
    DV_LOG->write_info(format("Папка программы: {}", base_path));

    texture_ = DV_TEXTURE_CACHE->get(base_path + "samples_data/textures/tile128.png");
    sprite_batch_ = make_unique<SpriteBatch>();
    font_ = make_unique<SpriteFont>(base_path + "samples_data/fonts/ubuntu-r_20_simple.fnt");

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

    StrUtf8 music_path = base_path + "samples_data/audio/in_the_field.mp3";
    music_ = Mix_LoadMUS(music_path.c_str());

    if (!music_)
        DV_LOG->write_error(format("App::start(): !music_ | {}", SDL_GetError()));

    if (Mix_PlayMusic(music_, -1) < 0)
        DV_LOG->write_error(format("App::start(): Mix_PlayMusic(...) < 0 | {}", SDL_GetError()));
}

bool App::handle_sdl_event(const SDL_Event& event)
{
    // Реагируем на закрытие приложения
    if (Application::handle_sdl_event(event))
        return true;

    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        on_key(event.key);
        return true;

    default:
        return false;
    }
}

void App::on_key(const SDL_KeyboardEvent& event_data)
{
    if (event_data.type == SDL_EVENT_KEY_DOWN && event_data.repeat == false
        && event_data.keysym.scancode == SDL_SCANCODE_ESCAPE)
    {
        should_exit_ = true;
    }
}

static float rotation = 0.f;
static StrUtf8 fps_text = "FPS: ?";
static float scale = 1.f;
static float scale_sign = 1.f;

void App::update(u64 ns)
{
    static u64 frame_counter = 0;
    static u64 time_counter = 0;

    ++frame_counter;
    time_counter += ns;

    // Обновляем fps_text каждые пол секунды
    if (time_counter >= SDL_NS_PER_SECOND / 2)
    {
        u64 fps = frame_counter * SDL_NS_PER_SECOND / time_counter;
        fps_text = format("FPS: {}", fps);
        frame_counter = 0;
        time_counter = 0;
    }

    rotation += ns * 0.000'000'000'1f;
    while (rotation >= 360.f)
        rotation -= 360.f;

    if (scale >= 2.f)
        scale_sign = -1.f;
    else if (scale <= 0.5f)
        scale_sign = 1.f;

    scale += scale_sign * ns * 0.0000000005f;

    //s_test_.update(ecs_, ns);
}

void App::draw()
{
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);

    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    sprite_batch_->triangle_.v0 = {{800.f, 0.f}, 0xFF00FF00};
    sprite_batch_->triangle_.v1 = {{800.f, 300.f}, 0xFF0000FF};
    sprite_batch_->triangle_.v2 = {{0.f, 300.f}, 0xFFFFFFFF};
    sprite_batch_->add_triangle();

    sprite_batch_->set_shape_color(0xFFFF0000);
    sprite_batch_->draw_triangle({400.f, 0.f}, {400.f, 600.f}, {0.f, 600.f});

    sprite_batch_->set_shape_color(0x90FFFF00);
    sprite_batch_->draw_rect({{screen_size.x - 400.f, screen_size.y - 200.f}, {300.f, 100.f}});

    sprite_batch_->draw_sprite(texture_, {100.f, 100.f});
    sprite_batch_->draw_sprite(texture_, {500.f, 100.f}, nullptr, 0x90FFFFFF, rotation);

    sprite_batch_->draw_string(fps_text, font_.get(), vec2{4.f, 1.f}, 0xFF000000);
    sprite_batch_->draw_string(fps_text, font_.get(), vec2{3.f, 0.f}, 0xFFFFFFFF);

    // Вращение строк вокруг экранной координаты
    StrUtf8 str = "QqWЙйр";
    u32 color = 0xFFFF9090;
    vec2 screen_origin{150.f, 115.f}; // Экранный origin
    vec2 screen_pos0 = vec2{100.f, 100.f}; // Желаемые экранные координаты строки при нулевом повороте
    vec2 screen_pos1 = vec2{200.f, 100.f};
    vec2 screen_pos2 = vec2{100.f, 130.f};
    vec2 screen_pos3 = vec2{200.f, 130.f};
    vec2 origin0 = screen_origin - screen_pos0; // Преобразуем экранный origin в локальный origin строки
    vec2 origin1 = screen_origin - screen_pos1;
    vec2 origin2 = screen_origin - screen_pos2;
    vec2 origin3 = screen_origin - screen_pos3;
    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin0, {scale, scale}, FlipModes::none);
    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin1, {scale, scale}, FlipModes::horizontally);
    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin2, {scale, scale}, FlipModes::vertically);
    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin3, {scale, scale}, FlipModes::both);
    sprite_batch_->draw_disk(screen_origin, 5.f, 8); // Рисуем экранный origin
    sprite_batch_->flush();
}

App::~App()
{
    Mix_FreeMusic(music_); // Проверка на nullptr не нужна
    music_ = nullptr;
}
