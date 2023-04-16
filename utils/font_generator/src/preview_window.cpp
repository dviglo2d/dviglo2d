#include "preview_window.hpp"

#include <dviglo/main/os_window.hpp>

#include <misc/cpp/imgui_stdlib.h>


PreviewWindow::PreviewWindow()
{
    // Возвращаемся к рендерингу в defaul
    // t framebuffer после создания fbo_
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PreviewWindow::render_text_to_texture(SpriteFont* font)
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
            vec2 shadow_pos = text_pos + shadow_offet;
            sprite_batch_->draw_string(text, font, shadow_pos, 0xFF000000);
        }

        sprite_batch_->draw_string(text, font, text_pos, ImGui::ColorConvertFloat4ToU32(text_color));

        sprite_batch_->flush();
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Возвращаемся к рендерингу в default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ivec2 screen_size;
    SDL_GetWindowSizeInPixels(DV_OS_WINDOW->window(), &screen_size.x, &screen_size.y);
    glViewport(0, 0, screen_size.x, screen_size.y);
}

void PreviewWindow::show(SpriteFont* font)
{
    namespace ig = ImGui;

    ig::SetNextWindowPos(ImVec2(10.f, 448.f), ImGuiCond_FirstUseEver);
    ig::SetNextWindowSize(ImVec2(1010.f, 500.f), ImGuiCond_FirstUseEver);
    if (ig::Begin("Превью шрифта"))
    {
        ig::PushItemWidth(-FLT_MIN); // Label элементов не используем
        {
            ig::TextUnformatted("Текст");
            ig::SameLine();
            ig::InputText("##text", &text);
        }
        ig::PopItemWidth();

        if (ig::BeginTable("##table", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings))
        {
            ig::TableNextColumn(); // Первая колонка

            ig::PushItemWidth(-FLT_MIN); // Label элементов не используем
            {
                ig::TextUnformatted("Позиция");
                ig::SameLine();
                ig::InputFloat2("##text_pos", &text_pos[0]);

                ig::TextUnformatted("Цвет");
                ig::SameLine();
                ig::ColorEdit4("##text_color", (f32*)&text_color, ImGuiColorEditFlags_AlphaPreview);
            }
            ig::PopItemWidth();

            ig::TableNextColumn(); // Вторая колонка

            ig::PushItemWidth(-FLT_MIN); // Label элементов не используем
            {
                ig::TextUnformatted("Тень");
                ig::SameLine();
                ig::Checkbox("##shadow", &shadow);

                ig::SameLine();
                ig::TextUnformatted("Смещение");
                ig::SameLine();
                ig::InputFloat2("##shadow_offet", &shadow_offet[0]);

                ig::TextUnformatted("Цвет фона");
                ig::SameLine();
                ig::ColorEdit4("##back_color", (f32*)&back_color, ImGuiColorEditFlags_AlphaPreview);
            }
            ig::PopItemWidth();

            ig::EndTable();
        }

        if (ig::BeginChild("##image", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
        {
            render_text_to_texture(font);
            Texture* texture = fbo_->texture();
            ImGui::Image(texture->gpu_object_name(), to_imvec2(texture->size()));
        }
        ig::EndChild(); // Нужно вызывать даже если BeginChild() вернул false
    }
    ig::End(); // Нужно вызывать даже если Begin() вернул false
}
