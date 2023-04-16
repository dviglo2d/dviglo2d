#pragma once

#include <dviglo/gl_utils/fbo.hpp>
#include <dviglo/graphics/sprite_batch.hpp>

#include <dv_imgui_misc.hpp>

using namespace dviglo;
using namespace glm;
using namespace std;


// Оформлено в виде класса, чтобы уничтожать FBO и SpriteBatch до уничтожения контекста OpenGL.
// Но экземпляр класса должен быть только один
class PreviewWindow
{
private:
    unique_ptr<Fbo> fbo_ = make_unique<Fbo>(ivec2(2048, 512));
    unique_ptr<SpriteBatch> sprite_batch_ = make_unique<SpriteBatch>();
    StrUtf8 text = "Съешь ещё этих мягких французских булок, да выпей чаю. 1234567890";
    vec2 text_pos{4.f, 1.f};
    ImVec4 text_color{1.f, 1.f, 1.f, 1.f};
    ImVec4 back_color{0.f, 0.5f, 1.f, 1.f};
    bool shadow = false;
    vec2 shadow_offet{2.f, 2.f};

    // Рендерит текст в текстуру
    void render_text_to_texture(SpriteFont* font);

public:
    PreviewWindow();

    void show(SpriteFont* font);
};
