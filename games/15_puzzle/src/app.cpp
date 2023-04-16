#include "app.hpp"

#include <dviglo/fs/fs_base.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/main/engine_params.hpp>


App::App(const vector<StrUtf8>& args)
    : Application(args)
{
}

void App::setup()
{
    engine_params::log_path = get_pref_path("dviglo2d", "mini_games") + "15_puzzle.log";
    engine_params::window_size = ivec2(720, 700);
    engine_params::window_title = "Пятнашки";
    engine_params::vsync = -1;
    engine_params::window_mode = WindowMode::windowed;
}

void App::start()
{
    StrUtf8 base_path = get_base_path();
    sprite_batch_ = make_unique<SpriteBatch>();
    spritesheet_ = DV_TEXTURE_CACHE->get(base_path + "15_puzzle_data/textures/spritesheet.png");
    my_font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path + "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 60));
    puzzle_logic_ = make_shared<PuzzleLogic>();
    puzzle_interface_ = make_shared<PuzzleInterface>(puzzle_logic_);
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

void App::update(u64 ns)
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
        sprite_batch_->draw_string("Победа!", my_font_.get(), vec2{200.f, 340.f});

    // Выводим остаток накопленных спрайтов
    sprite_batch_->flush();
}
