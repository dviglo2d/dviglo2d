#include "app.hpp"

#include <dv_random.hpp>
#include <dviglo/gl_utils/fbo.hpp>
#include <dviglo/main/os_window.hpp>
#include <dviglo/main/timer.hpp>

#include <iostream>

using namespace glm;


App::App()
{
    fs::path base_path = get_base_path();
    texture_ = DV_TEXTURE_CACHE->get(base_path / "engine_test_data/textures/tile128.png");
    font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 20));
    sprite_batch_ = make_unique<SpriteBatch>();

    // Возвращаемся к рендерингу в default framebuffer, так как текущий FBO меняется при генерации шрифта
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);
    glViewport(0, 0, screen_size.x, screen_size.y);
}

App::~App()
{
}

void App::handle_sdl_event(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        on_key(event.key);
        return;

    default:
        // Реагируем на закрытие приложения и изменение размера окна
        ApplicationBase::handle_sdl_event(event);
        return;
    }
}

void App::on_key(const SDL_KeyboardEvent& event_data)
{
    if (event_data.type == SDL_EVENT_KEY_DOWN && event_data.repeat == false
        && event_data.scancode == SDL_SCANCODE_ESCAPE)
    {
        should_exit_ = true;
    }
}

static StrUtf8 fps_text = "FPS: ?";

void App::update(i64 ns)
{
    static i64 frame_counter = 0;
    static i64 time_counter = 0;

    ++frame_counter;
    time_counter += ns;

    // Обновляем fps_text каждые пол секунды
    if (time_counter >= ns_per_s / 2)
    {
        i64 fps = frame_counter * ns_per_s / time_counter;
        fps_text = format("FPS: {}", fps);
        frame_counter = 0;
        time_counter = 0;
    }
}

void App::draw()
{
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);

    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true, false);

    for (i32 i = 0; i < 10000; ++i)
    {
        static Random rnd;
        i32 x = rnd.generate(-texture_->size().x / 2, screen_size.x - texture_->size().x / 2);
        i32 y = rnd.generate(-texture_->size().y / 2, screen_size.y - texture_->size().y / 2);
        sprite_batch_->draw_sprite(texture_.get(), vec2(x, y));
    }

    sprite_batch_->draw_string(fps_text, font_.get(), vec2(4.f, 1.f), 0xFF000000);
    sprite_batch_->draw_string(fps_text, font_.get(), vec2(3.f, 0.f), 0xFFFFFFFF);

    sprite_batch_->flush();
}
