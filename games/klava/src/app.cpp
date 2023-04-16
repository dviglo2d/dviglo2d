#include "app.hpp"

#include <dv_file.hpp>
#include <dv_log.hpp>
#include <dv_random.hpp>
#include <dviglo/gl_utils/gl_utils.hpp>
#include <dviglo/gl_utils/texture_cache.hpp>

using namespace glm;


App::App()
{
    fs::path base_path = get_base_path();
    sprite_batch_ = make_unique<SpriteBatch>();
    r_20_font_ = make_unique<SpriteFont>(SFSettingsSimple(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 20));
    r_60_colored_font_ = make_unique<SpriteFont>(SFSettingsOutlined(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 60, 0xFFFFFFFF, 0xFFFF9000, 4, 8));
    r_60_gray_font_ = make_unique<SpriteFont>(SFSettingsOutlined(base_path / "engine_test_data/fonts/ubuntu/Ubuntu-R.ttf", 60, 0xFF909090, 0xFF505050, 4, 8));

    // Возвращаемся к рендерингу в default framebuffer, так как текущий FBO меняется при генерации шрифта
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);
    glViewport(0, 0, screen_size.x, screen_size.y);

    // Загружаем слова из файла
    words_ = read_all_lines(base_path / "klava_data/words.txt", true);
    random_current_word();

    // Без этого событие SDL_EVENT_TEXT_INPUT не будет возникать
    SDL_StartTextInput(DV_OS_WINDOW->window());
}

App::~App()
{
}

static Random rnd;

void App::random_current_word()
{
    while (true)
    {
        // Избегаем повторов
        StrUtf8 new_word = words_[rnd.generate(0, static_cast<i32>(words_.size() - 1))];
        if (new_word != current_word_)
        {
            current_word_ = std::move(new_word);
            break;
        }
    }
}

void App::handle_sdl_event(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_TEXT_INPUT:
        on_text_input(event.text);
        return;

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

void App::on_text_input(const SDL_TextInputEvent& event_data)
{
    user_input_ += event_data.text;

    vector<c32> new_chars = to_utf32(event_data.text);
    vector<c32> current_word_chars = to_utf32(current_word_);
    vector<c32> correct_part_chars = to_utf32(correct_part_);

    for (c32 new_char : new_chars)
    {
        if (correct_part_chars.size() == current_word_chars.size())
            break;

        // Проверяем, что пользователь ввёл правильный символ
        if (new_char != current_word_chars[correct_part_chars.size()])
            continue;

        // Добавляем новый символ
        correct_part_chars.push_back(new_char);
    }

    correct_part_ = to_utf8(correct_part_chars);

    // Если пользователь ввёл слово целиком
    if (correct_part_.size() == current_word_.size())
    {
        if (user_input_ == current_word_) // Ввёл без ошибок
        {
            random_current_word();
            ++score_;
        }
        else
        {
            // Если пользователь ошибался, пусть вводит то же самое слово повторно
            score_ = 0;
        }


        user_input_ = "";
        correct_part_ = "";
        current_word_alpha_ = 0.f;
    }
}

void App::update(i64 ns)
{
    assert(current_word_alpha_ >= 0.f && current_word_alpha_ <= 1.f);

    // Около нуля альфа нарастает быстро, а около единицы - медленно
    if (current_word_alpha_ < 1.f)
        current_word_alpha_ += ns * 0.000000005f * (1.f - current_word_alpha_);

    if (current_word_alpha_ > 1.f)
        current_word_alpha_ = 1.f;
}

void App::draw()
{
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    sprite_batch_->prepare_ogl(true);

    // Центрируем слово
    vec2 current_word_size = sprite_batch_->measure_string(current_word_, r_60_gray_font_.get());
    vec2 word_pos = (screen_size - ivec2(current_word_size)) / 2; // В целых координатах, чтобы не было мыла

    // Часть слова, которую уже ввёл пользователь
    sprite_batch_->draw_string(correct_part_, r_60_colored_font_.get(), word_pos);
    vec2 correct_part_size = sprite_batch_->measure_string(correct_part_, r_60_colored_font_.get());

    // Остаток слова, который пользователь ещё не ввёл
    StrUtf8 remain = current_word_.substr(correct_part_.length());
    if (remain.length())
    {
        vec2 remain_pos(word_pos.x + correct_part_size.x, word_pos.y);
        u32 color = color_f32_to_u32(1.f, 1.f, 1.f, current_word_alpha_);
        sprite_batch_->draw_string(remain, r_60_gray_font_.get(), remain_pos, color);
    }

    // Если пользователь ошибался при вводе
    if (!starts_with(current_word_, user_input_))
    {
        vec2 pos(0.f, screen_size.y - r_20_font_->line_height());
        sprite_batch_->draw_string(user_input_, r_20_font_.get(), pos, 0xFF0000FF);
    }

    sprite_batch_->draw_string("Счёт: " + to_string(score_), r_20_font_.get(), vec2(10.f, 10.f));

    sprite_batch_->flush();
}
