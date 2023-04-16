#include "app.hpp"

#include <dviglo/gl_utils/fbo.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>
#include <dviglo/main/main_args.hpp>
#include <dviglo/main/os_window.hpp>
#include <dviglo/main/timer.hpp>

#include <iostream>

using namespace glm;


App::App()
{
    fs::path base_path = get_base_path();
    Log::writef_info("Командная строка: {}", join(DV_MAIN_ARGS->get(), " "));
    Log::writef_info("Папка программы: {}", base_path.string());

    texture_ = DV_TEXTURE_CACHE->get(base_path / "engine_test_data/textures/tile128.png");
    sprite_batch_ = make_unique<SpriteBatch>();

    font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 20));

#if 0
    fs::path font_path = base_path / "engine_test_data/fonts/ubuntu-r_20_simple.fnt";

    // Тестируем сохранение шрифта в файл
    font_->save(font_path);

    // TODO: Ещё надо создать файл ubuntu-r_20_simple_0.png.xml
    // <texture>
    //     <GL_TEXTURE_MIN_FILTER>GL_LINEAR_MIPMAP_LINEAR</GL_TEXTURE_MIN_FILTER>
    //     <GL_TEXTURE_MAG_FILTER>GL_LINEAR</GL_TEXTURE_MAG_FILTER>
    // </texture>

    // Тестируем загрузку шрифта из файла
    font_ = make_unique<SpriteFont>(font_path);
#endif

    fs::path music_path = base_path / "engine_test_data/audio/in_the_field.mp3";
    music_ = MIX_LoadAudio(DV_AUDIO->mixer(), music_path.string().c_str(), false);
    if (!music_)
        Log::writef_error("{} | !music_ | {}", DV_FUNC_SIG, SDL_GetError());

    music_track_ = MIX_CreateTrack(DV_AUDIO->mixer());
    if (!music_track_)
        Log::writef_error("{} | !music_track_ | {}", DV_FUNC_SIG, SDL_GetError());

    if (!MIX_SetTrackAudio(music_track_, music_))
        Log::writef_error("{} | !MIX_SetTrackAudio(...) | {}", DV_FUNC_SIG, SDL_GetError());

    SDL_PropertiesID options = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    if (!MIX_PlayTrack(music_track_, options))
        Log::writef_error("{} | !MIX_PlayTrack(...) | {}", DV_FUNC_SIG, SDL_GetError());
    SDL_DestroyProperties(options);

    // Рендерим в текстуру
    Fbo fbo(ivec2(256, 256));
    fbo.bind();
    glViewport(0, 0, fbo.texture()->size().x, fbo.texture()->size().y);
    glClearColor(1.f, 1.f, 0.f, 1.f); // Жёлтый фон
    glClear(GL_COLOR_BUFFER_BIT);
    sprite_batch_->prepare_ogl(true, true); // Отражаем вертикально
    sprite_batch_->draw_string("Отрендеренная сцена", font_.get(), vec2(4.f, 1.f), 0xFF000000);
    sprite_batch_->flush();
    rendered_scene_ = fbo.move_texture();
    rendered_scene_->bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    // Обработаем текстуру фильтром
    ShaderProgram* inverse_filter = DV_SHADER_CACHE->get(base_path / "engine_data/shaders/texture_shader.vert", base_path / "engine_test_data/shaders/inverse.frag");
    rendered_scene_->apply_shader(inverse_filter);
    rendered_scene_->bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    // Возвращаемся к рендерингу в default framebuffer
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
static f32 scale = 1.f;
static f32 scale_sign = 1.f;

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

    if (scale >= 2.f)
        scale_sign = -1.f;
    else if (scale <= 0.5f)
        scale_sign = 1.f;

    scale += scale_sign * ns * 0.0000000005f;
}

void App::draw()
{
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);

    glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true, false);

    sprite_batch_->triangle_.v0 = {{800.f, 0.f}, 0xFF00FF00};
    sprite_batch_->triangle_.v1 = {{800.f, 300.f}, 0xFF0000FF};
    sprite_batch_->triangle_.v2 = {{0.f, 300.f}, 0xFFFFFFFF};
    sprite_batch_->add_triangle();

    sprite_batch_->set_shape_color(0xFFFF0000);
    sprite_batch_->draw_triangle({400.f, 0.f}, {400.f, 600.f}, {0.f, 600.f});

    sprite_batch_->set_shape_color(0x90FFFF00);
    sprite_batch_->draw_rect({{screen_size.x - 400.f, screen_size.y - 200.f}, {300.f, 100.f}});

    sprite_batch_->draw_sprite(rendered_scene_.get(), {460.f, 400.f, 128.f, 128.f});

    Rect rect = sprite_batch_->measure_sprite(texture_.get(), {500.f, 100.f}, nullptr, rotation);
    sprite_batch_->set_shape_color(0x90FFFFFF);
    sprite_batch_->draw_rect(rect);
    sprite_batch_->draw_sprite(texture_.get(), {500.f, 100.f}, nullptr, 0x90FFFFFF, rotation);

    sprite_batch_->draw_string(fps_text, font_.get(), vec2(4.f, 1.f), 0xFF000000);
    sprite_batch_->draw_string(fps_text, font_.get(), vec2(3.f, 0.f), 0xFFFFFFFF);

    // Вращение строк вокруг экранной координаты
    StrUtf8 str = "QqWЙйр";
    u32 color = 0xFF000000;
    vec2 screen_origin{150.f, 115.f}; // Экранный origin
    vec2 screen_pos0 = vec2{100.f, 100.f}; // Желаемые экранные координаты строки при нулевом повороте
    vec2 screen_pos1 = vec2{200.f, 100.f};
    vec2 screen_pos2 = vec2{100.f, 130.f};
    vec2 screen_pos3 = vec2{200.f, 130.f};
    vec2 origin0 = screen_origin - screen_pos0; // Преобразуем экранный origin в локальный origin строки
    vec2 origin1 = screen_origin - screen_pos1;
    vec2 origin2 = screen_origin - screen_pos2;
    vec2 origin3 = screen_origin - screen_pos3;

    sprite_batch_->set_shape_color(0x90FFFFFF);
    Rect rect0 = sprite_batch_->measure_string_sprites(str, font_.get(), screen_origin, rotation, origin0, {scale, scale}, FlipModes::none);
    sprite_batch_->draw_rect(rect0);

    sprite_batch_->set_shape_color(0x90FF00FF);
    Rect rect1 = sprite_batch_->measure_string_sprites(str, font_.get(), screen_origin, rotation, origin1, {scale, scale}, FlipModes::horizontally);
    sprite_batch_->draw_rect(rect1);

    sprite_batch_->set_shape_color(0x9000FFFF);
    Rect rect2 = sprite_batch_->measure_string_sprites(str, font_.get(), screen_origin, rotation, origin2, {scale, scale}, FlipModes::vertically);
    sprite_batch_->draw_rect(rect2);

    sprite_batch_->set_shape_color(0x9000FF00);
    Rect rect3 = sprite_batch_->measure_string_sprites(str, font_.get(), screen_origin, rotation, origin3, {scale, scale}, FlipModes::both);
    sprite_batch_->draw_rect(rect3);

    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin0, {scale, scale}, FlipModes::none);
    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin1, {scale, scale}, FlipModes::horizontally);
    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin2, {scale, scale}, FlipModes::vertically);
    sprite_batch_->draw_string(str, font_.get(), screen_origin, color, rotation, origin3, {scale, scale}, FlipModes::both);

    sprite_batch_->set_shape_color(0x900000FF);
    sprite_batch_->draw_disk(screen_origin, 5.f, 8); // Рисуем экранный origin

    // Стыковка текста + выравнивание по правому нижнему краю окна
    vec2 rb_text_scale(3.25f, 5.75f); // rb - right bottom
    StrUtf8 rb_text1 = "Qqоbйooii";
    vec2 rb_text1_size = sprite_batch_->measure_string(rb_text1, font_.get(), rb_text_scale);
    StrUtf8 rb_text2 = "ii87il";
    vec2 rb_text2_size = sprite_batch_->measure_string(rb_text2, font_.get(), rb_text_scale);
    vec2 rb_text2_pos = vec2(screen_size) - rb_text2_size;
    vec2 rb_text1_pos(rb_text2_pos.x - rb_text1_size.x, rb_text2_pos.y);
    sprite_batch_->set_shape_color(0x7000FF00);
    sprite_batch_->draw_rect(Rect(rb_text1_pos, rb_text1_size));
    sprite_batch_->draw_rect(Rect(rb_text2_pos, rb_text2_size));
    sprite_batch_->draw_string(rb_text1, font_.get(), rb_text1_pos, 0xFFFFFFFF, 0.f, vec2(0.f, 0.f), rb_text_scale);
    sprite_batch_->draw_string(rb_text2, font_.get(), rb_text2_pos, 0xFFFFFFFF, 0.f, vec2(0.f, 0.f), rb_text_scale);

    sprite_batch_->flush();
}

App::~App()
{
}
