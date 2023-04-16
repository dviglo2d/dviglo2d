#include "app.hpp"

#include <dviglo/gl_utils/texture_cache.hpp>


App::App()
{
    fs::path base_path = get_base_path();
    sprite_batch_ = make_unique<SpriteBatch>();
    spritesheet_ = DV_TEXTURE_CACHE->get(base_path / "15_puzzle_data/textures/spritesheet.png");

    my_font_ = make_unique<SpriteFont>(SFSettingsOutlined(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 60,
                                                          0xFFFFFFFF, 0xFF90EE00, 4.f, 10));
    puzzle_logic_ = make_shared<PuzzleLogic>();
    puzzle_interface_ = make_shared<PuzzleInterface>(puzzle_logic_);

    // Возвращаемся к рендерингу в default framebuffer, так как текущий FBO меняется при генерации шрифта
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);
    glViewport(0, 0, screen_size.x, screen_size.y);
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

static constexpr vec2 new_game_size{238, 59}; // Размер кнопки "Новая игра"
static const Rect new_game_uv{{532, 17}, new_game_size}; // Участок текстуры с кнопкой
static constexpr vec2 new_game_pos{472, 10}; // Позиция кнопки на экране

void App::on_mouse_button(const SDL_MouseButtonEvent& event_data)
{
    if (event_data.type == SDL_EVENT_MOUSE_BUTTON_DOWN
        && event_data.button == SDL_BUTTON_LEFT)
    {
        vec2 pos{event_data.x, event_data.y};

        if (pos.x >= new_game_pos.x && pos.x < new_game_pos.x + new_game_size.x
            && pos.y >= new_game_pos.y && pos.y < new_game_pos.y + new_game_size.y)
        {
            puzzle_logic_->new_game();
            return;
        }

        puzzle_interface_->on_click({event_data.x, event_data.y});
    }
}

void App::update(i64 ns)
{
    (void)ns;
}

void App::draw()
{
    // Закрашиваем фон
    glClearColor(0.f, 0.3f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl();

    // Рисуем игровое поле
    puzzle_interface_->draw(sprite_batch_.get());

    // Рисуем кнопку "Новая игра"
    sprite_batch_->draw_sprite(spritesheet_.get(), new_game_pos, &new_game_uv);

    if (puzzle_logic_->check_win())
        sprite_batch_->draw_string("Победа!", my_font_.get(), vec2{216.f, 340.f});

    // Выводим накопленные спрайты
    sprite_batch_->flush();
}
