#include "app.hpp"

#include <dviglo/gl_utils/texture_cache.hpp>

using namespace glm;


App::App()
{
    fs::path base_path = get_base_path();
    sprite_batch_ = make_unique<SpriteBatch>();
    r_20_font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 20));

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

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        on_mouse_button(event.button);
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

void App::update(i64 ns)
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
