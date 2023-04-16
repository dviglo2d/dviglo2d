#include "app.hpp"

#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/main/engine_params.hpp>

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
    engine_params::log_path = get_pref_path("dviglo2d", "games") / "village.log";
    engine_params::window_size = ivec2(720, 700);
    engine_params::window_title = "Деревня";
    engine_params::vsync = -1;
    engine_params::window_mode = WindowMode::maximized;
}

void App::start()
{
    fs::path base_path = get_base_path();
    sprite_batch_ = make_unique<SpriteBatch>();
    r_20_font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 20));
    map_ = make_unique<Map>(10, 5);
    player_ = make_unique<Player>();
    spritesheet_ = DV_TEXTURE_CACHE->get(base_path / "village_data/tilemap_flat.png");

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
    (void)event_data;
}

void App::update(i64 ns)
{
    (void)ns;
}

void App::draw()
{
    glClearColor(0x3a / 255.f, 0x6f / 255.f, 0xa5 / 255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    for (i32 y = 0; y < map_->size().y; ++y)
    {
        for (i32 x = 0; x < map_->size().x; ++x)
        {
            const Tile& tile = (*map_)[y, x];
            constexpr i32 tile_size = 64;

            ivec2 tile_pos(x * tile_size, y * tile_size);

            // Считаем в целых координатах, чтобы не было ошибок округления
            IntRect tile_uv(1 * tile_size, 1 * tile_size, tile_size, tile_size);
            if (x == 0)
                tile_uv.pos.x -= tile_size;
            else if (x == map_->size().x - 1)
                tile_uv.pos.x += tile_size;

            if (y == 0)
                tile_uv.pos.y -= tile_size;
            else if (y == map_->size().y - 1)
                tile_uv.pos.y += tile_size;

            IntRect hummock_uv(4 * tile_size, 0 * tile_size, tile_size, tile_size);
            IntRect real_tile_uv(tile_uv);
            IntRect real_hummock_uv(hummock_uv);

            if (tile.type == TileType::sand)
            {
                // Смещение песочного набора тайлов относительно травянога набора тайлов в атласе
                vec2 offset(5 * tile_size, 0);

                real_tile_uv.pos += offset;
                real_hummock_uv.pos += offset;
            }

            Rect f_real_tile_uv(real_tile_uv);
            Rect f_real_hummock_uv(real_hummock_uv);

            sprite_batch_->draw_sprite(spritesheet_.get(), tile_pos, &f_real_tile_uv);

            if (tile.hummock)
                sprite_batch_->draw_sprite(spritesheet_.get(), tile_pos, &f_real_hummock_uv);
        }
    }

    player_->draw(*sprite_batch_);

    sprite_batch_->draw_string("TODO", r_20_font_.get(), vec2(10.f, 10.f));

    sprite_batch_->flush();
}
