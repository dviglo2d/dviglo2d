#include "app.hpp"

#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/main/timer.hpp>

using namespace glm;


App::App()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    my_subsystem_ = make_unique<MySubsystem>();
    assert(MY_SUBSYSTEM->get_value() == 1);

    fs::path base_path = get_base_path();
    texture_ = DV_TEXTURE_CACHE->get(base_path / "engine_test_data/textures/tile128.png");
    sprite_batch_ = make_unique<SpriteBatch>();
    r_20_font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 20));
    my_font_ = make_unique<SpriteFont>(SFSettingsOutlined(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 60, 0xFF00FFFF, 0xFF0000FF, 2));

    // Возвращаемся к рендерингу в default framebuffer, так как текущий FBO меняется при генерации шрифта
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);
    glViewport(0, 0, screen_size.x, screen_size.y);
}

App::~App()
{
    instance_ = nullptr;
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

static f32 rotation = 0.f;
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

    rotation += ns * 0.000'000'000'1f;
    while (rotation >= 360.f)
        rotation -= 360.f;
}

void App::draw()
{
    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    sprite_batch_->triangle_.v0 = {vec2(800.f, 0.f), 0xFF00FF00};
    sprite_batch_->triangle_.v1 = {vec2(800.f, 300.f), 0xFF0000FF};
    sprite_batch_->triangle_.v2 = {vec2(0.f, 300.f), 0xFFFFFFFF};
    sprite_batch_->add_triangle();

    sprite_batch_->set_shape_color(0xFFFF0000);
    sprite_batch_->draw_triangle(vec2(400.f, 0.f), vec2(400.f, 600.f), vec2(0.f, 600.f));

    sprite_batch_->set_shape_color(0x90FFFF00);
    sprite_batch_->draw_rect(Rect(300.f, 300.f, 300.f, 100.f));

    sprite_batch_->draw_sprite(texture_.get(), vec2(100.f, 100.f));
    sprite_batch_->draw_sprite(texture_.get(), vec2(500.f, 100.f), nullptr, 0xFFFFFFFF, rotation);

    vec2 mouse_pos;
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    sprite_batch_->draw_string("Привет!", my_font_.get(), mouse_pos);

    sprite_batch_->draw_string(fps_text, r_20_font_.get(), vec2(4.f, 1.f), 0xFF000000);
    sprite_batch_->draw_string(fps_text, r_20_font_.get(), vec2(3.f, 0.f), 0xFFFFFFFF);

    sprite_batch_->flush();
}
