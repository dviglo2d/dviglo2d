#include "app.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/main/engine_params.hpp>

#include <format>

using namespace glm;


App::App(const vector<StrUtf8>& args)
    : Application(args)
{
}

App::~App()
{
}

void App::setup()
{
    engine_params::log_path = get_pref_path("dviglo2d", "minimal_app") + "log.log";
    engine_params::window_size = {900, 700};
    engine_params::msaa_samples = 4; // При значении 8 крэшится на сервере ГитХаба в Линуксе
    engine_params::window_mode = WindowMode::windowed;
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    texture_ = DV_TEXTURE_CACHE->get(base_path + "engine_test_data/textures/tile128.png");
    sprite_batch_ = make_unique<SpriteBatch>();
    r_20_font_ = make_unique<SpriteFont>(base_path + "engine_test_data/fonts/ubuntu-r_20_simple.fnt");
    my_font_ = make_unique<SpriteFont>(base_path + "app_data/fonts/my_font.fnt");
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
        Application::handle_sdl_event(event);
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
}

void App::draw()
{
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
    sprite_batch_->draw_rect({300.f, 300.f, 300.f, 100.f});

    sprite_batch_->draw_sprite(texture_, {100.f, 100.f});
    sprite_batch_->draw_sprite(texture_, {500.f, 100.f}, nullptr, 0xFFFFFFFF, rotation);

    sprite_batch_->draw_string(fps_text, r_20_font_.get(), {4.f, 1.f}, 0xFF000000);
    sprite_batch_->draw_string(fps_text, r_20_font_.get(), {3.f, 0.f}, 0xFFFFFFFF);

    f32 mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    sprite_batch_->draw_string("Привет!", my_font_.get(), {mouse_x, mouse_y});

    sprite_batch_->flush();
}
