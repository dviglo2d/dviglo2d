#pragma once

#include <dviglo/graphics/sprite_batch.hpp>
#include <dviglo/main/application_base.hpp>

using namespace dviglo;
using namespace std;


class App final : public ApplicationBase
{
    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> r_20_font_;
    unique_ptr<SpriteFont> r_60_colored_font_;
    unique_ptr<SpriteFont> r_60_gray_font_;

    vector<StrUtf8> words_; // Словарь

    // Слово, которое должен набрать пользователь
    StrUtf8 current_word_;

    // Что ввёл пользователь
    StrUtf8 user_input_;

    // Корректно введённая часть слова
    StrUtf8 correct_part_;

    // Новое слово появляется прозрачным, а потом плавно появляется
    f32 current_word_alpha_ = 0.f;

    // Сколько слов подряд без ошибок
    i32 score_ = 0;

    // Меняет current_word_
    void random_current_word();

    void on_key(const SDL_KeyboardEvent& event_data);
    void on_mouse_button(const SDL_MouseButtonEvent& event_data);
    void on_text_input(const SDL_TextInputEvent& event_data);

public:
    App();
    ~App() final;

    void handle_sdl_event(const SDL_Event& event) final;
    void update(i64 ns) final;
    void draw() final;
};
