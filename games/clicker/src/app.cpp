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
    engine_params::log_path = get_pref_path("dviglo2d", "mini_games") + "clicker.log";
    engine_params::window_size = ivec2(720, 700);
    engine_params::msaa_samples = 4; // При значении 8 крэшится на сервере ГитХаба в Линуксе
    engine_params::window_title = "Кликер";
    engine_params::vsync = -1;
    engine_params::window_mode = WindowMode::windowed;
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    sprite_batch_ = make_unique<SpriteBatch>();
    r_20_font_ = make_unique<SpriteFont>(base_path + "engine_test_data/fonts/ubuntu-r_20_simple.fnt");
}

void App::handle_sdl_event(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        on_key(event.key);
        return;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        on_mouse_button(event.button);
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

void App::on_mouse_button(const SDL_MouseButtonEvent& event_data)
{
    if (event_data.type == SDL_EVENT_MOUSE_BUTTON_DOWN
        && event_data.button == SDL_BUTTON_LEFT)
    {
        gold_ += power_;
    }

    else if (event_data.type == SDL_EVENT_MOUSE_BUTTON_DOWN
             && event_data.button == SDL_BUTTON_RIGHT)
    {
        // Цена апгрейда равна силе клика
        if (gold_ >= power_)
        {
            gold_ -= power_;
            ++power_;
        }
    }
}

void App::update(u64 ns)
{
    (void)ns;
}

void App::draw()
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    StrUtf8 help_str = "ЛКМ - добывать золото, ПКМ - усилить кирку";
    sprite_batch_->draw_string(help_str, r_20_font_.get(), vec2(10.f, 10.f));

    StrUtf8 power_str = "Сила нажатия: " + power_.to_string() + " (= цене апгрейда)";
    sprite_batch_->draw_string(power_str, r_20_font_.get(), vec2(10.f, 40.f));

    StrUtf8 gold_str = "Золота: " + gold_.to_string();
    sprite_batch_->draw_string(gold_str, r_20_font_.get(), vec2(300.f, 300.f));

    sprite_batch_->flush();
}
