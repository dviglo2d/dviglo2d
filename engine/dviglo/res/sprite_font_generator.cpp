// Copyright (c) the Dviglo project
// License: MIT

// Конструкторы SpriteFont, которые используют FreeType

#include "sprite_font.hpp"

#include "freetype.hpp"
#include "freetype_utils.hpp"

#include "../fs/file.hpp"
#include "../fs/log.hpp"
#include "../gl_utils/gl_utils.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../main/timer.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

using namespace glm;
using namespace std;


namespace dviglo
{

// Обёртка над FT_Face
class FreeTypeFace
{
private:
    FT_Face face_ = nullptr; // Это указатель

    // Данные нужно держать в памяти до вызова FT_Done_Face()
    vector<byte> data;

public:
    FreeTypeFace(const SFSettings& font_settings)
    {
        // FT_New_Face() ожидает путь в кодировке ANSI, поэтому испольузем FT_New_Memory_Face()
        data = read_all_data(font_settings.src_path);
        if (data.empty())
        {
            DV_LOG->writef_error("{} | data.empty()", DV_FUNCSIG);
            return;
        }

        FT_Error error = FT_New_Memory_Face(DV_FREETYPE->library(),
                                            r_cast<const FT_Byte*>(data.data()),
                                            s_cast<FT_Long>(data.size()), 0, &face_);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_New_Memory_Face(...) error {}", DV_FUNCSIG, error);
            return;
        }

        error = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Select_Charmap(...) error {}", DV_FUNCSIG, error);
            return;
        }

        // Шрифт может содержать несколько начертаний, но используется только первое
        if (face_->num_faces != 1)
            DV_LOG->writef_warning("{} | face_->num_faces != 1 | {}", DV_FUNCSIG, face_->num_faces);

        // Реальная высота текста отличается от запрошенной
        error = FT_Set_Pixel_Sizes(face_, 0, font_settings.height);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Set_Pixel_Sizes(...) error {}", DV_FUNCSIG, error);
            return;
        }
    }

    ~FreeTypeFace()
    {
        if (face_)
        {
            FT_Error error = FT_Done_Face(face_);
            if (error)
                DV_LOG->writef_error("{} | FT_Done_Face(...) error {}", DV_FUNCSIG, error);
        }
    }

    // Запрещаем копирование
    FreeTypeFace(const FreeTypeFace&) = delete;
    FreeTypeFace& operator =(const FreeTypeFace&) = delete;

    // Запрещаем перемещение
    FreeTypeFace(FreeTypeFace&&) = delete;
    FreeTypeFace& operator =(FreeTypeFace&&) = delete;

    FT_Face get() const { return face_; }
};


// Копирует FT_Bitmap в grayscale Image
static Image to_image(const FT_Bitmap& bitmap)
{
    Image ret(bitmap.width, bitmap.rows, 1);

    // Если изображение монохромное
    if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
    {
        // В монохромном изображении один пиксель занимает 1 бит (не байт).
        // pitch - это число байт, занимаемых одной линией изображения
        for (i32 y = 0; y < ret.size().y; ++y)
        {
            u8* src = bitmap.buffer + bitmap.pitch * y;
            u8* dest = ret.data() + y * ret.size().x;

            for (i32 x = 0; x < ret.size().x; ++x)
            {
                // 1) В одном байте умещается 8 пикселей. x >> 3 эквивалентно делению
                //    на 8 (0b1000 превращается в 0b1). Так мы получаем байт, в котором находится пиксель
                // 2) x & 7 - это остаток от деления на 8. Допустим x = 12 = 0b1100
                //    0b1100 & 0b0111 = 0b100 = 4. Так мы получаем номер бита внутри байта.
                // 3) 0x80 == 0b10000000. Единица внутри этого числа сдвигается на номер бита
                //    и побитовой операцией определяется значение бита
                dest[x] = (src[x >> 3] & (0x80 >> (x & 7))) ? 255 : 0;
            }
        }
    }
    else // Grayscale
    {
        // В grayscale изображении каждый пиксель занимает один байт,
        // а значит pitch эквивалентен width
        for (i32 i = 0; i < ret.size().x * ret.size().y; ++i)
            ret.data()[i] = bitmap.buffer[i];
    }

    return ret;
}


struct RenderedGlyph
{
    // Для стилей Simple и Contour изображение grayscale,
    // а для Outlined - rgba
    unique_ptr<Image> image;

    RenderedGlyph() = default;

    // Запрещаем копирование
    RenderedGlyph(const RenderedGlyph&) = delete;
    RenderedGlyph& operator=(const RenderedGlyph&) = delete;

    // Разрешаем перемещение
    RenderedGlyph(RenderedGlyph&&) = default;
    RenderedGlyph& operator=(RenderedGlyph&&) = default;

    // Символ в кодировке UTF-32
    u32 code_point = 0;

    // Смещение при выводе на экран
    glm::ivec2 offset{0, 0};

    // Расстояние между origin текущего глифа и origin следующего глифа
    i32 x_advance = 0;

    i32 page = 0;

    // Область в текстуре
    IntRect rect = IntRect::zero;
};

// Расширяет и размывает grayscale Image, если нужно
static void apply_blur(RenderedGlyph& rendered_glyph, const i32 blur_radius)
{
    assert(blur_radius >= 0);
    assert(rendered_glyph.image->num_components() == 1);

    if (!blur_radius)
        return;

    // Расширенное изображение
    Image new_image(rendered_glyph.image->size() + blur_radius * 2, rendered_glyph.image->num_components());
    // Вставляем исходное изображение в центр расширенного
    new_image.paste(*rendered_glyph.image, ivec2(blur_radius));
    new_image.blur_triangle(blur_radius);
    *rendered_glyph.image = std::move(new_image);

    // Размытый текст предназначен для создания тени от неразмытого текста
    rendered_glyph.offset -= blur_radius;
}

SpriteFont::SpriteFont(const SFSettingsSimple& settings, i64* generation_time_ms)
{
    i64 begin_time_ms = get_ticks_ms();

    FreeTypeFace face(settings);

    FT_UInt glyph_index;
    FT_ULong char_code = FT_Get_First_Char(face.get(), &glyph_index);
    vector<RenderedGlyph> rendered_glyphs;
    rendered_glyphs.reserve(face.get()->num_glyphs);
    vector<stbrp_rect> rects;
    rects.reserve(face.get()->num_glyphs);
    vector<shared_ptr<Image>> pages;

    i32 vec_ind = 0;

    const i32 padding = 2;

    while (glyph_index != 0)
    {
        // При загрузке глифа сразу же его рендерим
        FT_Int32 load_flags = FT_LOAD_RENDER;

        // По умолчанию рендерится в режиме FT_RENDER_MODE_NORMAL.
        // Перезаписываем, если сглаживание отключено
        if (!settings.anti_aliasing)
        {
            load_flags |= FT_LOAD_MONOCHROME;  // 1 бит на пиксель
            load_flags |= FT_LOAD_TARGET_MONO; // Алгоритм хинтига
        }

        RenderedGlyph rendered_glyph;

        // Результат будет помещён в слот
        FT_Error error = FT_Load_Glyph(face.get(), glyph_index, load_flags);

        if (error)
        {
            DV_LOG->writef_error("SpriteFont::SpriteFont(SFSettingsSimple ...) | FT_Load_Glyph(...) | {}", error);
        }
        else
        {
            rendered_glyph.code_point = char_code;

            // Забираем данные из слота (FT_Face->glyph)
            rendered_glyph.image = make_unique<Image>(to_image(face.get()->glyph->bitmap));
            rendered_glyph.offset.x = round_to_pixels(face.get()->glyph->metrics.horiBearingX);
            rendered_glyph.offset.y = round_to_pixels(face.get()->size->metrics.ascender - face.get()->glyph->metrics.horiBearingY);
            rendered_glyph.x_advance = round_to_pixels(face.get()->glyph->metrics.horiAdvance);

            apply_blur(rendered_glyph, settings.blur_radius);

            stbrp_rect r{};
            r.id = vec_ind;
            r.w = rendered_glyph.image->width() + padding * 2;
            r.h = rendered_glyph.image->height() + padding * 2;
            rects.push_back(r);

            rendered_glyphs.push_back(std::move(rendered_glyph));

            char_code = FT_Get_Next_Char(face.get(), char_code, &glyph_index);
            ++vec_ind;
        }
    }

    stbrp_context pack_context;
    i32 num_nodes = settings.texture_size.x;
    vector<stbrp_node> nodes(num_nodes);

    while (rects.size())
    {
        shared_ptr<Image> current_page = make_shared<Image>(settings.texture_size, 1);
        stbrp_init_target(&pack_context, settings.texture_size.x, settings.texture_size.y, nodes.data(), num_nodes);
        stbrp_pack_rects(&pack_context, rects.data(), (i32)rects.size());

        for (size_t i = 0; i < rects.size();)
        {
            stbrp_rect& rect = rects[i];

            if (rect.was_packed)
            {
                current_page->paste(*rendered_glyphs[rect.id].image, ivec2(rect.x + padding, rect.y + padding));
                rendered_glyphs[rect.id].page = (i32)pages.size();
                rendered_glyphs[rect.id].rect.pos = ivec2(rect.x, rect.y) + padding;
                rendered_glyphs[rect.id].rect.size = ivec2(rect.w, rect.h) - padding * 2;
                assert(rendered_glyphs[rect.id].rect.size.x == rendered_glyphs[rect.id].image->size().x);

                // Удаляем упакованный прямоугольник из списка, путём перемещения в конец
                rect = std::move(rects.back());
                rects.pop_back();
            }
            else
            {
                ++i;
            }
        }

        pages.push_back(current_page);
    }

    line_height_ = round_to_pixels(face.get()->size->metrics.height);

    for (const RenderedGlyph& rendered_glyph : rendered_glyphs)
    {
        Glyph glyph;
        glyph.page = rendered_glyph.page;
        glyph.rect = rendered_glyph.rect;
        glyph.advance_x = rendered_glyph.x_advance;
        glyph.offset = rendered_glyph.offset;

        glyphs_[rendered_glyph.code_point] = glyph;
    }

    for (shared_ptr<Image> page : pages)
    {
        shared_ptr<Image> colored_page = make_shared<Image>(page->to_rgba(settings.color));
        shared_ptr<Texture> page_tex = make_shared<Texture>(colored_page, true);
        DV_TEXTURE_CACHE->add(page_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures_.push_back(page_tex);
    }

    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_info("SpriteFont::SpriteFont(const FontSettingsSimple&) | {} | Generated in {} ms", settings.src_path, duration_ms);
}

RenderedGlyph render_glyph_contour(FT_Face face, const SFSettingsContour& font_settings)
{
    RenderedGlyph ret;

    assert(font_settings.blur_radius >= 0);

    // Реднерим глиф
    FT_Glyph glyph;

    FT_Error error = FT_Get_Glyph(face->glyph, &glyph);

    if (error)
    {
        DV_LOG->writef_error("render_glyph_simpe(): FT_Get_Glyph() error | {}", error);
        return ret;
    }

    // Отличает от simple
    FT_Stroker stroker;
    FT_Stroker_New(glyph->library, &stroker);
    FT_Stroker_Set(stroker, FT_Fixed(font_settings.thickness * (64 / 2)), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    //FT_Glyph_StrokeBorder(&glyph, stroker, false, true); // Снаруж или внутри
    FT_Glyph_Stroke(&glyph, stroker, true);
    FT_Stroker_Done(stroker);
    // Конец


    FT_Render_Mode render_mode = font_settings.anti_aliasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
    error = FT_Glyph_To_Bitmap(&glyph, render_mode, nullptr, 1);

    if (error)
    {
        DV_LOG->writef_error("render_glyph_simpe(): FT_Glyph_To_Bitmap() error | {}", error);
        FT_Done_Glyph(glyph);
        return ret;
    }

    FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    ret.image = make_unique<Image>(to_image(bitmap_glyph->bitmap));
    FT_Done_Glyph(glyph);


    ret.offset.x = round_to_pixels(face->glyph->metrics.horiBearingX);
    ret.offset.y = round_to_pixels(face->size->metrics.ascender - face->glyph->metrics.horiBearingY);
    ret.x_advance = face->glyph->metrics.horiAdvance >> 6;

    // Отличаетися от simple
    // Глиф стал больше примерно на половину толщины контура в каждую сторону.
    // Необходимо вручную модифицировать метрики.
    // См. примечание https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Glyph_Stroke
    ret.x_advance += (f32)font_settings.thickness;
    // Конец

    apply_blur(ret, font_settings.blur_radius);

    return ret;
}

SpriteFont::SpriteFont(const SFSettingsContour& settings, i64* generation_time_ms)
{
    i64 begin_time_ms = get_ticks_ms();

    FreeTypeFace face(settings);

    FT_UInt glyph_index;
    FT_ULong char_code = FT_Get_First_Char(face.get(), &glyph_index);
    vector<RenderedGlyph> rendered_glyphs;
    rendered_glyphs.reserve(face.get()->num_glyphs);
    vector<stbrp_rect> rects;
    rects.reserve(face.get()->num_glyphs);
    vector<shared_ptr<Image>> pages;

    i32 vec_ind = 0;

    const i32 padding = 2;

    while (glyph_index != 0)
    {
        // Алгоритм хинтига
        FT_Int32 load_flags = settings.anti_aliasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;

        FT_Load_Glyph(face.get(), glyph_index, load_flags);
        RenderedGlyph rendered_glyph = render_glyph_contour(face.get(), settings);
        rendered_glyph.code_point = char_code;

        stbrp_rect r{};
        r.id = vec_ind;
        r.w = rendered_glyph.image->width() + padding * 2;
        r.h = rendered_glyph.image->height() + padding * 2;
        rects.push_back(r);

        rendered_glyphs.push_back(std::move(rendered_glyph));

        char_code = FT_Get_Next_Char(face.get(), char_code, &glyph_index);

        ++vec_ind;
    }

    stbrp_context pack_context;
    i32 num_nodes = settings.texture_size.x;
    vector<stbrp_node> nodes(num_nodes);

    while (rects.size())
    {
        shared_ptr<Image> current_page = make_shared<Image>(settings.texture_size, 1);
        stbrp_init_target(&pack_context, settings.texture_size.x, settings.texture_size.y, nodes.data(), num_nodes);
        stbrp_pack_rects(&pack_context, rects.data(), (i32)rects.size());

        for (size_t i = 0; i < rects.size();)
        {
            stbrp_rect& rect = rects[i];

            if (rect.was_packed)
            {
                current_page->paste(*rendered_glyphs[rect.id].image, ivec2(rect.x + padding, rect.y + padding));
                rendered_glyphs[rect.id].page = (i32)pages.size();
                rendered_glyphs[rect.id].rect.pos = ivec2(rect.x, rect.y) + padding;
                rendered_glyphs[rect.id].rect.size = ivec2(rect.w, rect.h) - padding * 2;
                assert(rendered_glyphs[rect.id].rect.size.x == rendered_glyphs[rect.id].image->size().x);

                // Удаляем упакованный прямоугольник из списка, путём перемещения в конец
                rect = std::move(rects.back());
                rects.pop_back();
            }
            else
            {
                ++i;
            }
        }

        pages.push_back(current_page);
    }

    line_height_ = round_to_pixels(face.get()->size->metrics.height);

    // Отличаетися от simple
    // Глиф стал больше примерно на половину толщины контура в каждую сторону.
    // Необходимо вручную модифицировать метрики.
    // См. примечание https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Glyph_Stroke
    line_height_ += (f32)settings.thickness;
    // Конец

    for (const RenderedGlyph& rendered_glyph : rendered_glyphs)
    {
        Glyph glyph;
        glyph.page = rendered_glyph.page;
        glyph.rect = rendered_glyph.rect;
        glyph.advance_x = rendered_glyph.x_advance;
        glyph.offset = rendered_glyph.offset;

        glyphs_[rendered_glyph.code_point] = glyph;
    }

    for (shared_ptr<Image> page : pages)
    {
        shared_ptr<Image> colored_page = make_shared<Image>(page->to_rgba(settings.color));
        shared_ptr<Texture> page_tex = make_shared<Texture>(colored_page, true);
        DV_TEXTURE_CACHE->add(page_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures_.push_back(page_tex);
    }

    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_info("SpriteFont::SpriteFont(const FontSettingsSimple&) | {} | Generated in {} ms", settings.src_path, duration_ms);
}


RenderedGlyph render_glyph_outlined(FT_Face face, const SFSettingsOutlined& settings)
{
    RenderedGlyph ret;

    //assert(font_settings.blur_radius >= 0);

    // Реднерим глиф
    FT_Glyph glyph;
    FT_BitmapGlyph bitmapGlyph;


    // Реднерим внутренний глиф обычным способом
    FT_Get_Glyph(face->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

    // Можно вычислить и до ренгедринга
    ret.offset.x = round_to_pixels(face->glyph->metrics.horiBearingX);
    ret.offset.y = round_to_pixels(face->size->metrics.ascender - face->glyph->metrics.horiBearingY);
    ret.x_advance = face->glyph->metrics.horiAdvance >> 6;
    ret.x_advance += i32(settings.outline_thickness * 2);

    // Используем манипулятор просто как место временного хранения.
    Image normalGlyph(to_image(bitmapGlyph->bitmap));

    int normalGlyphLeft = bitmapGlyph->left;
    int normalGlyphTop = bitmapGlyph->top;
    FT_Done_Glyph(glyph);

    // Рендерим раздутый глиф.
    FT_Get_Glyph(face->glyph, &glyph);
    FT_Stroker stroker;
    FT_Stroker_New(glyph->library, &stroker);
    FT_Stroker_Set(stroker, FT_Fixed(settings.outline_thickness * 64), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
    FT_Stroker_Done(stroker);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    Image inflatedGlyph(to_image(bitmapGlyph->bitmap));
    int inflatedGlyphLeft = bitmapGlyph->left;
    int inflatedGlyphTop = bitmapGlyph->top;
    FT_Done_Glyph(glyph);

    // Смещение нормального изображения относительно раздутого.
    // Оно не всегда равно толщине обводки. Поэтому вычисляем так.
    int deltaX = normalGlyphLeft - inflatedGlyphLeft;
    int deltaY = inflatedGlyphTop - normalGlyphTop;

    if (settings.outline_blur_radius > 0)
    {
        Image new_image(inflatedGlyph.size() + settings.outline_blur_radius * 2,
            inflatedGlyph.num_components());

        // Вставляем в центр расширенного изображения
        new_image.paste(inflatedGlyph, ivec2(settings.outline_blur_radius, settings.outline_blur_radius));

        // TODO: Изображение может быть нулевого размера
        new_image.blur_triangle(settings.outline_blur_radius);

        inflatedGlyph = std::move(new_image);

        ret.offset.x -= settings.outline_blur_radius;
        ret.offset.y -= settings.outline_blur_radius;
    }

    if (settings.outline_blur_radius > 0)
    {
        // Переделать
        deltaX += settings.outline_blur_radius;
        deltaY += settings.outline_blur_radius;
    }

    // Специальный случай - цвета внутри и снаружи совпадают. При этом не выводим внутренний глиф.
// При размытии обводки внутренний глиф будет видно, даже если их цвет одинаковый.
// Это на случай, если будет нужна размытая тень для глифа с обводкой.

    ret.image = make_unique<Image>(inflatedGlyph.to_rgba(settings.outline_color));

    if (settings.main_color != settings.outline_color)
    {
        // Накладываем нормальный глиф на раздутый.
        // Это не альфа-блендинг. Тут пиксели нормального (внтуренннего) глифа перезаписывают
        // пиксели раздутого глифа. Но учитывается альфа крайних полупрозрачных пикселей.
        for (int y = 0; y < normalGlyph.size().y; ++y)
        {
            for (int x = 0; x < normalGlyph.size().x; ++x)
            {
                u32 backColor;
                memcpy(&backColor, ret.image->pixel_ptr(x + deltaX, y + deltaY), 4);

                u32 frontColor = settings.main_color;

                u32 mask = normalGlyph.pixel_ptr(x, y)[0];
                u32 result_r = get_r(frontColor) * mask + get_r(backColor) * (0xFFu - mask);
                u32 result_g = get_g(frontColor) * mask + get_g(backColor) * (0xFFu - mask);
                u32 result_b = get_b(frontColor) * mask + get_b(backColor) * (0xFFu - mask);
                u32 result_a = get_a(frontColor) * mask + get_a(backColor) * (0xFFu - mask);
                u32 result_color = to_rgba(result_r / 0xFF, result_g / 0xFF, result_b / 0xFF, result_a / 0xFF);
                memcpy(ret.image->pixel_ptr(x + deltaX, y + deltaY), &result_color, 4);

                /*
                float mask = normalGlyph.GetPixel(x, y);
                float r = mask * mainColor_.r_ + backColor.r_ * (1.0f - mask);
                float g = mask * mainColor_.g_ + backColor.g_ * (1.0f - mask);
                float b = mask * mainColor_.b_ + backColor.b_ * (1.0f - mask);
                float a = mask * mainColor_.a_ + backColor.a_ * (1.0f - mask);
                resultImage->SetPixel(x + deltaX, y + deltaY, Color(r, g, b, a));
                */
            }
        }
    }

    return ret;
}

SpriteFont::SpriteFont(const SFSettingsOutlined& settings, i64* generation_time_ms)
{
    i64 begin_time_ms = get_ticks_ms();

    FreeTypeFace face(settings);

    FT_UInt glyph_index;
    FT_ULong char_code = FT_Get_First_Char(face.get(), &glyph_index);
    vector<RenderedGlyph> rendered_glyphs;
    rendered_glyphs.reserve(face.get()->num_glyphs);
    vector<stbrp_rect> rects;
    rects.reserve(face.get()->num_glyphs);
    vector<shared_ptr<Image>> pages;

    i32 vec_ind = 0;

    const i32 padding = 2;

    while (glyph_index != 0)
    {
        // Алгоритм хинтига
        FT_Int32 load_flags = settings.anti_aliasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;

        FT_Load_Glyph(face.get(), glyph_index, load_flags);
        RenderedGlyph rendered_glyph = render_glyph_outlined(face.get(), settings);
        rendered_glyph.code_point = char_code;

        stbrp_rect r{};
        r.id = vec_ind;
        r.w = rendered_glyph.image->width() + padding * 2;
        r.h = rendered_glyph.image->height() + padding * 2;
        rects.push_back(r);

        rendered_glyphs.push_back(std::move(rendered_glyph));

        char_code = FT_Get_Next_Char(face.get(), char_code, &glyph_index);

        ++vec_ind;
    }

    stbrp_context pack_context;
    i32 num_nodes = settings.texture_size.x;
    vector<stbrp_node> nodes(num_nodes);

    while (rects.size())
    {
        shared_ptr<Image> current_page = make_shared<Image>(settings.texture_size, 4);
        stbrp_init_target(&pack_context, settings.texture_size.x, settings.texture_size.y, nodes.data(), num_nodes);
        stbrp_pack_rects(&pack_context, rects.data(), (i32)rects.size());

        for (size_t i = 0; i < rects.size();)
        {
            stbrp_rect& rect = rects[i];

            if (rect.was_packed)
            {
                current_page->paste(*rendered_glyphs[rect.id].image, ivec2(rect.x + padding, rect.y + padding));
                rendered_glyphs[rect.id].page = (i32)pages.size();
                rendered_glyphs[rect.id].rect.pos = ivec2(rect.x, rect.y) + padding;
                rendered_glyphs[rect.id].rect.size = ivec2(rect.w, rect.h) - padding * 2;
                assert(rendered_glyphs[rect.id].rect.size.x == rendered_glyphs[rect.id].image->size().x);

                // Удаляем упакованный прямоугольник из списка, путём перемещения в конец
                rect = std::move(rects.back());
                rects.pop_back();
            }
            else
            {
                ++i;
            }
        }

        pages.push_back(current_page);
    }

    line_height_ = round_to_pixels(face.get()->size->metrics.height);

    // Отличаетися от simple
    // Глиф стал больше примерно на половину толщины контура в каждую сторону.
    // Необходимо вручную модифицировать метрики.
    // См. примечание https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Glyph_Stroke
    line_height_ += i32(settings.outline_thickness * 2);
    // Конец

    for (const RenderedGlyph& rendered_glyph : rendered_glyphs)
    {
        Glyph glyph;
        glyph.page = rendered_glyph.page;
        glyph.rect = rendered_glyph.rect;
        glyph.advance_x = rendered_glyph.x_advance;
        glyph.offset = rendered_glyph.offset;

        glyphs_[rendered_glyph.code_point] = glyph;
    }

    for (shared_ptr<Image> page : pages)
    {
        shared_ptr<Image> colored_page = page; // Изображение уже преобразовано в rgba
        shared_ptr<Texture> page_tex = make_shared<Texture>(colored_page, true);
        DV_TEXTURE_CACHE->add(page_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures_.push_back(page_tex);
    }

    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_info("SpriteFont::SpriteFont(const FontSettingsSimple&) | {} | Generated in {} ms", settings.src_path, duration_ms);
}


} // namespace dviglo
