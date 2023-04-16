// Copyright (c) the Dviglo project
// License: MIT

// Конструкторы SpriteFont, которые используют FreeType

#include "sprite_font.hpp"

#include "freetype.hpp"
#include "freetype_utils.hpp"

#include "../fs/file.hpp"
#include "../fs/log.hpp"
#include "../fs/path.hpp"
#include "../gl_utils/gl_utils.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../main/timer.hpp"
#include "../math/math.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

using namespace glm;
using namespace std;


namespace dviglo
{

// RAII-обёртка для FT_Face
class FreeTypeFace
{
private:
    FT_Face face_ = nullptr; // Это указатель

    // Данные нужно держать в памяти до вызова FT_Done_Face(...)
    vector<byte> data;

    void done()
    {
        if (face_)
        {
            FT_Error error = FT_Done_Face(face_);
            if (error)
                DV_LOG->writef_error("{} | FT_Done_Face(...) error {}", DV_FUNCSIG, error);

            face_ = nullptr;
        }
    }

public:
    FreeTypeFace(const SFSettings& font_settings)
    {
        // FT_New_Face(...) ожидает путь в кодировке ANSI, поэтому испольузем FT_New_Memory_Face(...)
        data = read_all_data(font_settings.src_path);
        if (data.empty())
        {
            DV_LOG->writef_error("{} | data.empty()", DV_FUNCSIG);
            return;
        }

        // Шрифт может содержать несколько начертаний, но используется только первое
        FT_Error error = FT_New_Memory_Face(DV_FREETYPE->library(),
                                            reinterpret_cast<const FT_Byte*>(data.data()),
                                            static_cast<FT_Long>(data.size()),
                                            0, &face_);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_New_Memory_Face(...) error {}", DV_FUNCSIG, error);
            done();
            return;
        }

        error = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Select_Charmap(...) error {}", DV_FUNCSIG, error);
            done();
            return;
        }

        // Шрифт может содержать несколько начертаний, но используется только первое
        if (face_->num_faces != 1)
            DV_LOG->writef_warning("{} | face_->num_faces != 1 | {}", DV_FUNCSIG, face_->num_faces);

        // Реальная высота текста чаще всего отличается от запрошенной
        error = FT_Set_Pixel_Sizes(face_, 0, font_settings.height);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Set_Pixel_Sizes(...) error {}", DV_FUNCSIG, error);
            done();
            return;
        }
    }

    ~FreeTypeFace()
    {
        done();
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

    for (i32 y = 0; y < ret.size().y; ++y)
    {
        // pitch - это число байт, занимаемых одной линией изображения
        u8* src = bitmap.buffer + bitmap.pitch * y;

        u8* dest = ret.data() + ret.size().x * y;

        for (i32 x = 0; x < ret.size().x; ++x)
        {
            // Если пиксель занимает 1 бит
            if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
            {
                // Индекс байта, в котором находится пиксель.
                // В одном байте хранится 8 пикселей, поэтому делим x на 8
                i32 byte_index = x >> 3; // x / 8

                // Индекс бита внутри байта
                i32 bit_index = x & 7; // x % 8 == x & 0b111

                u8 pixel_mask = 0b1000'0000 >> bit_index;
                dest[x] = (src[byte_index] & pixel_mask) ? 255 : 0;
            }
            else // Пиксель занимает 1 байт (grayscale)
            {
                dest[x] = src[x];
            }
        }
    }

    return ret;
}


struct RenderedGlyph
{
    // Для стилей Simple и Contour изображение grayscale,
    // а для Outlined - rgba
    unique_ptr<Image> image;

    // Символ в кодировке UTF-32
    c32 code_point = 0;

    SFGlyph sf_glyph;

    // TODO: Удалить
    RenderedGlyph() = default;

    // Забирает уже отрендеренный глиф из слота (FT_Face->glyph)
    RenderedGlyph(const FT_ULong char_code, const FreeTypeFace& face)
    {
        const FT_GlyphSlot slot = face.get()->glyph; // Это указатель

        code_point = static_cast<c32>(char_code);
        image = make_unique<Image>(to_image(slot->bitmap));
        sf_glyph.offset.x = round_to_pixels(slot->metrics.horiBearingX);
        sf_glyph.offset.y = round_to_pixels(face.get()->size->metrics.ascender - slot->metrics.horiBearingY);
        sf_glyph.advance_x = round_to_pixels(slot->metrics.horiAdvance);
    }

    // Запрещаем копирование
    RenderedGlyph(const RenderedGlyph&) = delete;
    RenderedGlyph& operator=(const RenderedGlyph&) = delete;

    // Разрешаем перемещение
    RenderedGlyph(RenderedGlyph&&) noexcept = default;
    RenderedGlyph& operator=(RenderedGlyph&&) noexcept = default;

    // Расширяет и размывает grayscale Image, если нужно
    void blur(const i32 blur_radius)
    {
        assert(blur_radius >= 0);
        assert(image->num_components() == 1);

        if (!blur_radius)
            return;

        // Расширенное изображение
        Image new_image(image->size() + blur_radius * 2, image->num_components());
        // Вставляем исходное изображение в центр расширенного
        new_image.paste(*image, ivec2(blur_radius));
        new_image.blur_triangle(blur_radius);
        *image = std::move(new_image);
        // Размытый текст предназначен для создания тени от неразмытого текста
        sf_glyph.offset -= blur_radius;
    }
};


// Одноразовый объект, так как метод pack() меняет исходные данные
class GlyphPacker
{
    static constexpr i32 padding = 2;

    vector<RenderedGlyph> rendered_glyphs_;
    vector<stbrp_rect> rects_;

#ifndef NDEBUG
    bool packed_ = false;
#endif

public:
    GlyphPacker(FT_Long num_glyphs)
    {
        rendered_glyphs_.reserve(num_glyphs);
        rects_.reserve(num_glyphs);
    }

    // Запрещаем копирование
    GlyphPacker(const GlyphPacker&) = delete;
    GlyphPacker& operator =(const GlyphPacker&) = delete;

    // Запрещаем перемещение
    GlyphPacker(GlyphPacker&&) = delete;
    GlyphPacker& operator =(GlyphPacker&&) = delete;

    const vector<RenderedGlyph>& rendered_glyphs() const { return rendered_glyphs_; }

    void add(RenderedGlyph&& rendered_glyph)
    {
        stbrp_rect r{};
        r.id = static_cast<i32>(rendered_glyphs_.size()); // Индекс в векторе rendered_glyphs
        r.w = rendered_glyph.image->width() + padding * 2;
        r.h = rendered_glyph.image->height() + padding * 2;
        rects_.push_back(r);

        rendered_glyphs_.push_back(std::move(rendered_glyph));
    }

    // Возвращает набор атласов. Меняет исходные данные
    vector<shared_ptr<Image>> pack(const ivec2 texture_size)
    {
        // Проверяем, что метод не вызывается повторно
        assert(!packed);

#ifndef NDEBUG
        packed_ = true;
#endif

        assert(rendered_glyphs_.size() > 0);

        vector<shared_ptr<Image>> ret;

        stbrp_context pack_context;
        const i32 num_nodes = texture_size.x;
        vector<stbrp_node> nodes(num_nodes);

        while (rects_.size())
        {
            shared_ptr<Image> current_atlas = make_shared<Image>(texture_size, rendered_glyphs_[0].image->num_components());
            stbrp_init_target(&pack_context, texture_size.x, texture_size.y, nodes.data(), num_nodes);
            stbrp_pack_rects(&pack_context, rects_.data(), (i32)rects_.size());

            for (size_t i = 0; i < rects_.size();)
            {
                stbrp_rect& rect = rects_[i];

                if (rect.was_packed)
                {
                    current_atlas->paste(*rendered_glyphs_[rect.id].image, ivec2(rect.x + padding, rect.y + padding));
                    rendered_glyphs_[rect.id].sf_glyph.atlas_index = (i32)ret.size();
                    rendered_glyphs_[rect.id].sf_glyph.rect.pos = ivec2(rect.x, rect.y) + padding;
                    rendered_glyphs_[rect.id].sf_glyph.rect.size = ivec2(rect.w, rect.h) - padding * 2;
                    assert(rendered_glyphs_[rect.id].sf_glyph.rect.size.x == rendered_glyphs_[rect.id].image->size().x);

                    // Удаляем упакованный прямоугольник из списка, путём перемещения в конец
                    rect = std::move(rects_.back());
                    rects_.pop_back();
                }
                else
                {
                    ++i;
                }
            }

            ret.push_back(current_atlas);
        }

        return ret;
    }
};


SpriteFont::SpriteFont(const SFSettingsSimple& settings, i64* generation_time_ms)
{
    i64 begin_time_ms = get_ticks_ms();

    FreeTypeFace face(settings);

    FT_UInt glyph_index;
    FT_ULong char_code = FT_Get_First_Char(face.get(), &glyph_index);

    GlyphPacker glyph_packer(face.get()->num_glyphs);

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

        // Результат будет помещён в слот
        FT_Error error = FT_Load_Glyph(face.get(), glyph_index, load_flags);

        if (error)
        {
            DV_LOG->writef_error("{} | FT_Load_Glyph(...) | {}", DV_FUNCSIG, error);
            continue;
        }

        RenderedGlyph rendered_glyph(char_code, face);
        rendered_glyph.blur(settings.blur_radius);
        glyph_packer.add(std::move(rendered_glyph));

        char_code = FT_Get_Next_Char(face.get(), char_code, &glyph_index);
    }

    vector<shared_ptr<Image>> atlases = glyph_packer.pack(settings.texture_size);

    for (const RenderedGlyph& rendered_glyph : glyph_packer.rendered_glyphs())
        glyphs_[rendered_glyph.code_point] = rendered_glyph.sf_glyph;

    for (shared_ptr<Image> atlas : atlases)
    {
        shared_ptr<Image> colored_atlas = make_shared<Image>(atlas->to_rgba(settings.color));
        shared_ptr<Texture> atlas_tex = make_shared<Texture>(colored_atlas, true);
        DV_TEXTURE_CACHE->add(atlas_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures_.push_back(atlas_tex);
    }

    // face.get()->size->metrics.height - это расстояние между базовыми линиями.
    // Смотри коммент к FT_FaceRec_ в freetype.h
    line_height_ = round_to_pixels(face.get()->size->metrics.height);

    src_info_ = "file: " + get_file_name(settings.src_path);
    src_info_ += ", style: simple, ";
    src_info_ += "height: " + to_string(settings.height);

    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_debug("{} | {} | Generated in {} ms", DV_FUNCSIG, settings.src_path, duration_ms);
}

RenderedGlyph render_glyph_contour(FT_Face face, const SFSettingsContour& font_settings)
{
    RenderedGlyph ret;

    assert(font_settings.blur_radius >= 0);

    FT_Glyph glyph;
    FT_Error error = FT_Get_Glyph(face->glyph, &glyph);

    if (error)
    {
        DV_LOG->writef_error("{} | FT_Get_Glyph(...) error | {}", DV_FUNCSIG, error);
        return ret;
    }

    FT_Stroker stroker;
    FT_Stroker_New(glyph->library, &stroker);
    FT_Stroker_Set(stroker, static_cast<FT_Fixed>(font_settings.thickness * (64 / 2)), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    FT_Glyph_Stroke(&glyph, stroker, 1); // Заменяем глиф контуром глифа
    FT_Stroker_Done(stroker);

    FT_Render_Mode render_mode = font_settings.anti_aliasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
    error = FT_Glyph_To_Bitmap(&glyph, render_mode, nullptr, 1);

    if (error)
    {
        DV_LOG->writef_error("{} | FT_Glyph_To_Bitmap(...) error | {}", DV_FUNCSIG, error);
        FT_Done_Glyph(glyph);
        return ret;
    }

    FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    ret.image = make_unique<Image>(to_image(bitmap_glyph->bitmap));
    FT_Done_Glyph(glyph);

    ret.sf_glyph.offset.x = round_to_pixels(face->glyph->metrics.horiBearingX);
    ret.sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - face->glyph->metrics.horiBearingY);

    // См. примечание https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Glyph_Stroke
    ret.sf_glyph.advance_x = round_to_i32(to_f32(face->glyph->metrics.horiAdvance) + font_settings.thickness);

    ret.blur(font_settings.blur_radius);

    return ret;
}

SpriteFont::SpriteFont(const SFSettingsContour& settings, i64* generation_time_ms)
{
    i64 begin_time_ms = get_ticks_ms();

    FreeTypeFace face(settings);

    FT_UInt glyph_index;
    FT_ULong char_code = FT_Get_First_Char(face.get(), &glyph_index);

    GlyphPacker glyph_packer(face.get()->num_glyphs);

    while (glyph_index != 0)
    {
        // Алгоритм хинтига
        FT_Int32 load_flags = settings.anti_aliasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;

        FT_Load_Glyph(face.get(), glyph_index, load_flags);
        RenderedGlyph rendered_glyph = render_glyph_contour(face.get(), settings);
        rendered_glyph.code_point = char_code;

        glyph_packer.add(std::move(rendered_glyph));

        char_code = FT_Get_Next_Char(face.get(), char_code, &glyph_index);
    }

    vector<shared_ptr<Image>> atlases = glyph_packer.pack(settings.texture_size);

    for (const RenderedGlyph& rendered_glyph : glyph_packer.rendered_glyphs())
        glyphs_[rendered_glyph.code_point] = rendered_glyph.sf_glyph;

    for (shared_ptr<Image> atlas : atlases)
    {
        shared_ptr<Image> colored_atlas = make_shared<Image>(atlas->to_rgba(settings.color));
        shared_ptr<Texture> atlas_tex = make_shared<Texture>(colored_atlas, true);
        DV_TEXTURE_CACHE->add(atlas_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures_.push_back(atlas_tex);
    }

    // См. примечание https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Glyph_Stroke
    line_height_ = round_to_i32(to_f32(face.get()->size->metrics.height) + settings.thickness);

    src_info_ = "file: " + get_file_name(settings.src_path);
    src_info_ += ", style: contour, ";
    src_info_ += "height: " + to_string(settings.height);

    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_debug("{} | {} | Generated in {} ms", DV_FUNCSIG, settings.src_path, duration_ms);
}


RenderedGlyph render_glyph_outlined(FT_Face face, const SFSettingsOutlined& settings)
{
    RenderedGlyph ret;

    //assert(font_settings.blur_radius >= 0);

    // Реднерим глиф
    FT_Glyph glyph;
    FT_BitmapGlyph bitmap_glyph;

    FT_Render_Mode render_mode = settings.anti_aliasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;

    // Реднерим внутренний глиф обычным способом
    FT_Get_Glyph(face->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, render_mode, nullptr, true);
    bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph);

    // Можно вычислить и до ренгедринга
    ret.sf_glyph.offset.x = round_to_pixels(face->glyph->metrics.horiBearingX);
    ret.sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - face->glyph->metrics.horiBearingY);
    ret.sf_glyph.advance_x = round_to_pixels(face->glyph->metrics.horiAdvance);
    ret.sf_glyph.advance_x += i32(settings.outline_thickness * 2);

    // Используем манипулятор просто как место временного хранения.
    Image normal_glyph = to_image(bitmap_glyph->bitmap);

    i32 normal_glyph_left = bitmap_glyph->left;
    i32 normal_glyph_top = bitmap_glyph->top;
    FT_Done_Glyph(glyph);

    // Рендерим раздутый глиф.
    FT_Get_Glyph(face->glyph, &glyph);
    FT_Stroker stroker;
    FT_Stroker_New(glyph->library, &stroker);
    FT_Stroker_Set(stroker, FT_Fixed(settings.outline_thickness * 64), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
    FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
    FT_Stroker_Done(stroker);
    FT_Glyph_To_Bitmap(&glyph, render_mode, nullptr, true);
    bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    Image inflated_glyph(to_image(bitmap_glyph->bitmap));
    i32 inflated_glyph_left = bitmap_glyph->left;
    i32 inflated_glyph_top = bitmap_glyph->top;
    FT_Done_Glyph(glyph);

    // Смещение нормального изображения относительно раздутого.
    // Оно не всегда равно толщине обводки. Поэтому вычисляем так.
    i32 delta_x = normal_glyph_left - inflated_glyph_left;
    i32 delta_y = inflated_glyph_top - normal_glyph_top;

    if (settings.outline_blur_radius > 0)
    {
        Image new_image(inflated_glyph.size() + settings.outline_blur_radius * 2, inflated_glyph.num_components());

        // Вставляем в центр расширенного изображения
        new_image.paste(inflated_glyph, ivec2(settings.outline_blur_radius, settings.outline_blur_radius));

        // TODO: Изображение может быть нулевого размера
        new_image.blur_triangle(settings.outline_blur_radius);

        inflated_glyph = std::move(new_image);

        ret.sf_glyph.offset.x -= settings.outline_blur_radius;
        ret.sf_glyph.offset.y -= settings.outline_blur_radius;
    }

    if (settings.outline_blur_radius > 0)
    {
        // Переделать
        delta_x += settings.outline_blur_radius;
        delta_y += settings.outline_blur_radius;
    }

    ret.image = make_unique<Image>(inflated_glyph.to_rgba(settings.outline_color));

    if (settings.render_inner)
    {
        // Накладываем нормальный глиф на раздутый
        for (i32 y = 0; y < normal_glyph.size().y; ++y)
        {
            for (i32 x = 0; x < normal_glyph.size().x; ++x)
            {
                u32 back_color;
                memcpy(&back_color, ret.image->pixel_ptr(x + delta_x, y + delta_y), 4);

                u32 front_color = settings.main_color;

                u32 mask = normal_glyph.pixel_ptr(x, y)[0];
                u32 result_r = get_r(front_color) * mask + get_r(back_color) * (0xFFu - mask);
                u32 result_g = get_g(front_color) * mask + get_g(back_color) * (0xFFu - mask);
                u32 result_b = get_b(front_color) * mask + get_b(back_color) * (0xFFu - mask);
                u32 result_a = get_a(front_color) * mask + get_a(back_color) * (0xFFu - mask);
                u32 result_color = to_rgba(static_cast<u8>(result_r / 0xFF), static_cast<u8>(result_g / 0xFF), static_cast<u8>(result_b / 0xFF), static_cast<u8>(result_a / 0xFF));
                memcpy(ret.image->pixel_ptr(x + delta_x, y + delta_y), &result_color, 4);
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

    GlyphPacker glyph_packer(face.get()->num_glyphs);

    while (glyph_index != 0)
    {
        // Алгоритм хинтига
        FT_Int32 load_flags = settings.anti_aliasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;

        FT_Load_Glyph(face.get(), glyph_index, load_flags);
        RenderedGlyph rendered_glyph = render_glyph_outlined(face.get(), settings);
        rendered_glyph.code_point = char_code;
        glyph_packer.add(std::move(rendered_glyph));

        char_code = FT_Get_Next_Char(face.get(), char_code, &glyph_index);
    }

    vector<shared_ptr<Image>> atlases = glyph_packer.pack(settings.texture_size);

    for (const RenderedGlyph& rendered_glyph : glyph_packer.rendered_glyphs())
        glyphs_[rendered_glyph.code_point] = rendered_glyph.sf_glyph;

    for (shared_ptr<Image> atlas : atlases)
    {
        shared_ptr<Image> colored_atlas = atlas; // Изображение уже преобразовано в rgba
        shared_ptr<Texture> atlas_tex = make_shared<Texture>(colored_atlas, true);
        DV_TEXTURE_CACHE->add(atlas_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        textures_.push_back(atlas_tex);
    }

    // См. примечание https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Glyph_Stroke
    line_height_ = round_to_i32(to_f32(face.get()->size->metrics.height) + settings.outline_thickness * 2);

    src_info_ = "file: " + get_file_name(settings.src_path);
    src_info_ += ", style: outlined, ";
    src_info_ += "height: " + to_string(settings.height);

    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_debug("{} | {} | Generated in {} ms", DV_FUNCSIG, settings.src_path, duration_ms);
}

} // namespace dviglo
