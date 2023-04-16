#pragma once

#include <dviglo/gl_utils/fbo.hpp>
#include <dviglo/graphics/sprite_batch.hpp>
#include <dviglo/main/os_window.hpp>

#include <dv_imgui_utils.hpp>
#include <misc/cpp/imgui_stdlib.h>

using namespace dviglo;
using namespace glm;
using namespace std;


class PreviewWindow
{
private:
    unique_ptr<Fbo> fbo_ = make_unique<Fbo>(ivec2(2048, 512));
    unique_ptr<SpriteBatch> sprite_batch_ = make_unique<SpriteBatch>();
    StrUtf8 test_text = "Съешь ещё этих мягких французских булок, да выпей чаю. 1234567890";
    bool shadow = false;
    ImVec4 back_color{0.f, 0.f, 0.f, 0.f};
    vec2 pos{4.f, 1.f};
    vec2 shadow_offet{1.f, 1.f};

    // Рендерит текст в текстуру
    void render_text_to_texture(SpriteFont* font)
    {
        fbo_->bind();
        glViewport(0, 0, fbo_->texture()->size().x, fbo_->texture()->size().y);

        // Очищаем фон
        glClearColor(back_color.x, back_color.y, back_color.z, back_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        if (font)
        {
            sprite_batch_->prepare_ogl(true, true); // Отражаем вертикально

            if (shadow)
            {
                vec2 shadow_pos = pos + shadow_offet;
                sprite_batch_->draw_string(test_text, font, shadow_pos, 0xFF000000);
            }

            sprite_batch_->draw_string(test_text, font, pos, 0xFFFFFFFF);

            sprite_batch_->flush();
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // Возвращаемся к рендерингу в default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ivec2 screen_size;
        SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);
        glViewport(0, 0, screen_size.x, screen_size.y);
    }

public:
    void show(SpriteFont* font)
    {
        namespace ig = ImGui;

        ig::SetNextWindowPos(ImVec2(10.f, 364.f), ImGuiCond_FirstUseEver);
        ig::SetNextWindowSize(ImVec2(1010.f, 500.f), ImGuiCond_FirstUseEver);
        ig::Begin("Превью шрифта");

        ig::PushItemWidth(-FLT_MIN); // Label элементов не используем
        {
            ig::TextUnformatted("Текст");
            ig::SameLine();
            ig::InputText("##test_text", &test_text);

            ig::TextUnformatted("Позиция текста");
            ig::SameLine();
            ig::InputFloat2("##text_pos1", &pos[0]);

            ig::TextUnformatted("Тень");
            ig::SameLine();
            ig::Checkbox("##test_text_shadow", &shadow);
            ig::SameLine();
            ig::TextUnformatted("Смещение тени");
            ig::SameLine();
            ig::InputFloat2("##shaw_off", &shadow_offet[0]);

            ig::TextUnformatted("Цвет фона");
            ig::SameLine();
            ig::ColorEdit4("##test_text_image_back_color", (f32*)&back_color, ImGuiColorEditFlags_AlphaPreview);
        }
        ig::PopItemWidth(); // Label элементов не используем

        ig::BeginChild("##test_text_image", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        {
            render_text_to_texture(font);
            Texture* texture = fbo_->texture();
            ImGui::Image(texture->gpu_object_name(), to_imvec2(texture->size()));
        }
        ig::EndChild();

        ig::End();
    }
};
