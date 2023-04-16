#include "app.hpp"

#include <dviglo/gl_utils/gl_utils.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>

using namespace glm;


App::App()
{
    fs::path base_path = get_base_path();
    sprite_batch_ = make_unique<SpriteBatch>();
    r_20_font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 20));
    map_ = make_unique<Map>();
    player_ = make_unique<Player>();

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
    (void)event_data;
}

void App::update(i64 ns)
{
    player_->update(ns);
}

void App::draw()
{
    glClearColor(0x3a / 255.f, 0x6f / 255.f, 0xa5 / 255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    ivec2 viewport_size = get_viewport().size;

    // Камера центрируется на ступнях персонажа: при отрисовке все координаты смещаем
    vec2 pos_offset = -player_->pos + vec2(viewport_size / 2);

    // Рисуем карту
    fs::path base_path = get_base_path();
    shared_ptr<Texture> tilemap_flat_png = DV_TEXTURE_CACHE->get(base_path / "survivors_data/tilemap_flat.png");

    for (i32 y = 0; y < map_->size().y; ++y)
    {
        for (i32 x = 0; x < map_->size().x; ++x)
        {
            const Tile& tile = (*map_)[y, x];
            ivec2 tile_pos(x * Tile::size, y * Tile::size);
            vec2 screen_pos = vec2(tile_pos) + pos_offset;
            sprite_batch_->draw_sprite(tilemap_flat_png.get(), screen_pos, &tile.uv);
        }
    }

    // В каком тайле ступни персонажа
    i32 player_tile_y = player_->pos_to_tile(*map_, player_->pos).y;

    // Рисуем скалы дальше персонажа
    shared_ptr<Texture> tilemap_elevation_png = DV_TEXTURE_CACHE->get(base_path / "survivors_data/tilemap_elevation.png");
    for (i32 y = 0; y < map_->size().y && y <= player_tile_y; ++y)
    {
        for (i32 x = 0; x < map_->size().x; ++x)
        {
            const Tile& tile = (*map_)[y, x];
            if (tile.obstacle)
            {
                // Спрайт занимает два тайла в высоту
                ivec2 sprite_pos(x * Tile::size, (y - 1) * Tile::size);
                vec2 screen_pos = vec2(sprite_pos) + pos_offset;
                sprite_batch_->draw_sprite(tilemap_elevation_png.get(), screen_pos, &tile.obstacle_uv);
            }
        }
    }

    // Рисуем персонажа (в центре экрана находятся ступни персонажа)
    shared_ptr<Texture> warrior_blue_png = DV_TEXTURE_CACHE->get(base_path / "survivors_data/warrior_blue.png");
    Rect player_uv(0.f, 0.f, 192.f, 192.f);
    vec2 player_sprite_pos(viewport_size.x / 2  - 96, viewport_size.y / 2 - 128); // На спрайте много пустого места
    sprite_batch_->draw_sprite(warrior_blue_png.get(), player_sprite_pos, &player_uv);

    // Рисуем коллайдер персонажа в центре экрана
    sprite_batch_->set_shape_color(0x90FFFFFF);
    vec2 player_collider_pos(viewport_size / 2 - player_->half_size);
    sprite_batch_->draw_rect(Rect(player_collider_pos, static_cast<vec2>(player_->half_size * 2)));

    // Рисуем камни ближе персонажа
    for (i32 y = player_tile_y; y < map_->size().y; ++y)
    {
        for (i32 x = 0; x < map_->size().x; ++x)
        {
            const Tile& tile = (*map_)[y, x];
            if (tile.obstacle)
            {
                // Спрайт занимает два тайла в высоту
                ivec2 sprite_pos(x * Tile::size, (y - 1) * Tile::size);
                vec2 screen_pos = vec2(sprite_pos) + pos_offset;
                sprite_batch_->draw_sprite(tilemap_elevation_png.get(), screen_pos, &tile.obstacle_uv);
            }
        }
    }

    sprite_batch_->draw_string("TODO", r_20_font_.get(), vec2(10.f, 10.f));

    sprite_batch_->flush();
}
