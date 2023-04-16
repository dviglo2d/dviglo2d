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
#include "../std_utils/scope_guard.hpp"

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
    FT_Face face_ = nullptr;

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
    FreeTypeFace(const SFSettings& settings)
    {
        // FT_New_Face(...) ожидает путь в кодировке ANSI, поэтому испольузем FT_New_Memory_Face(...)
        data = read_all_data(settings.src_path);
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

        if (face_->num_faces != 1)
            DV_LOG->writef_warning("{} | face_->num_faces != 1 | {}", DV_FUNCSIG, face_->num_faces);

        error = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Select_Charmap(...) error {}", DV_FUNCSIG, error);
            done();
            return;
        }

        // Реальная высота текста чаще всего отличается от запрошенной
        error = FT_Set_Pixel_Sizes(face_, 0, settings.height);
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


struct RenderedGlyph
{
private:
    // Копирует FT_Bitmap в grayscale Image
    static Image to_image(const FT_Bitmap& bitmap)
    {
        if (bitmap.width * bitmap.rows == 0)
            return Image();

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

    // Расширяет и размывает grayscale Image, если нужно.
    // Функция модифицирует sf_glyph.offset
    void blur(const i32 blur_radius)
    {
        assert(blur_radius >= 0);
        assert(image.num_components() == 1 || image.empty());

        if (!blur_radius)
            return;

        // Расширенное изображение
        //Image new_image(image.size() + blur_radius * 2, image.num_components());
        Image new_image(image.size() + blur_radius * 2, 1);

        // Вставляем исходное изображение в центр расширенного
        new_image.paste(image, ivec2(blur_radius));

        new_image.blur_triangle(blur_radius);
        image = std::move(new_image);

        // Размытый текст предназначен для создания тени от неразмытого текста
        sf_glyph.offset -= blur_radius;
    }

    void error_image()
    {
        assert(code_point);

        // TODO: Пурпурное изображение в случае ошибки
    }

public:
    Image image;

    // Символ в кодировке UTF-32
    c32 code_point = 0;

    SFGlyph sf_glyph;

    // Рендерит глиф обычным способом
    RenderedGlyph(const FT_Face face, const SFSettingsSimple& settings,
                  const FT_UInt glyph_index, const FT_ULong char_code)
    {
        assert(settings.blur_radius >= 0);

        code_point = static_cast<c32>(char_code);

        FT_Int32 load_flags = FT_LOAD_NO_BITMAP;
        // При загрузке глифа сразу же его рендерим
        load_flags |= FT_LOAD_RENDER;

        // По умолчанию рендерится в режиме FT_RENDER_MODE_NORMAL.
        // Меняем режим, если сглаживание отключено
        if (!settings.anti_aliasing)
        {
            load_flags |= FT_LOAD_MONOCHROME;  // 1 бит на пиксель
            load_flags |= FT_LOAD_TARGET_MONO; // Алгоритм хинтига
        }

        // Загружаем глиф в слот и сразу рендерим
        FT_Error error = FT_Load_Glyph(face, glyph_index, load_flags);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Load_Glyph(...) | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }

        const FT_GlyphSlot slot = face->glyph; // Это указатель

        // Импортируем grayscale глиф из слота
        image = to_image(slot->bitmap);

        sf_glyph.offset.x = round_to_pixels(slot->metrics.horiBearingX);
        sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - slot->metrics.horiBearingY);
        sf_glyph.advance_x = round_to_pixels(slot->metrics.horiAdvance);

        // Размываем grayscale глиф (модифицирует sf_glyph.offset)
        blur(settings.blur_radius);

        // Окрашиваем
        image = image.to_rgba(settings.color);
    }

    // Рендерит контур глифа
    RenderedGlyph(const FT_Face face, const SFSettingsContour& settings,
                  const FT_UInt glyph_index, const FT_ULong char_code)
    {
        assert(settings.blur_radius >= 0);

        code_point = static_cast<c32>(char_code);

        FT_Int32 load_flags = FT_LOAD_NO_BITMAP;
        // Алгоритм хинтига
        load_flags |= settings.anti_aliasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;

        // Загружаем глиф в слот
        FT_Error error = FT_Load_Glyph(face, glyph_index, load_flags);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Load_Glyph(...) | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }

        const FT_GlyphSlot slot = face->glyph; // Это указатель

        // Импортируем глиф из слота
        FT_Glyph glyph = nullptr;
        error = FT_Get_Glyph(slot, &glyph);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Get_Glyph(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем глиф в конце функции
        ScopeGuard done_glyph = [&glyph] { FT_Done_Glyph(glyph); };

        // Выделяем память для обводчика
        FT_Stroker stroker = nullptr;
        error = FT_Stroker_New(glyph->library, &stroker);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Stroker_New(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем обводчик в конце функции
        ScopeGuard done_stroker = [&stroker] { FT_Stroker_Done(stroker); };

        // Радиус - половина толщины (делим на 2).
        // Радиус должен быть в формате 26.6 (умножаем на 64)
        FT_Fixed radius = static_cast<FT_Fixed>(settings.thickness * 32);
        // Настраиваем обводчик
        FT_Stroker_Set(stroker, radius, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

        // Заменяем глиф контуром глифа
        error = FT_Glyph_Stroke(&glyph, stroker, 1);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Glyph_Stroke(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }

        // Рендерим глиф
        FT_Render_Mode render_mode = settings.anti_aliasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
        error = FT_Glyph_To_Bitmap(&glyph, render_mode, nullptr, 1);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Glyph_To_Bitmap(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }

        // Преобразуем отрендеренный глиф в grayscale Image
        FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph); // Это указатель
        image = to_image(bitmap_glyph->bitmap);

        // См. коммент SFGlyph::offset
        sf_glyph.offset.x = round_to_pixels(slot->metrics.horiBearingX);
        sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - slot->metrics.horiBearingY);

        // См. примечание https://freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#ft_glyph_stroke
        sf_glyph.advance_x = round_to_i32(to_f32(slot->metrics.horiAdvance) + settings.thickness);

        // Размываем grayscale глиф. Функция модифицирует sf_glyph.offset
        blur(settings.blur_radius);

        // Окрашиваем
        image = image.to_rgba(settings.color);
    }

    // Рендерит глиф с обводкой
    RenderedGlyph(const FT_Face face, const SFSettingsOutlined& settings,
                  const FT_UInt glyph_index, const FT_ULong char_code)
    {
        assert(settings.outline_blur_radius >= 0);

        code_point = static_cast<c32>(char_code);

        FT_Int32 load_flags = FT_LOAD_NO_BITMAP;
        // Алгоритм хинтига
        load_flags |= settings.anti_aliasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;
        // Загружаем глиф в слот
        FT_Error error = FT_Load_Glyph(face, glyph_index, load_flags);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Load_Glyph(...) | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }

        const FT_GlyphSlot slot = face->glyph; // Это указатель

        // ======================= Рендерим раздутый глиф =======================

        // Импортируем глиф из слота
        FT_Glyph normal_glyph = nullptr;
        error = FT_Get_Glyph(slot, &normal_glyph);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Get_Glyph(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем глиф в конце функции
        ScopeGuard done_normal_glyph = [&normal_glyph] { FT_Done_Glyph(normal_glyph); };

        // Выделяем память для обводчика
        FT_Stroker stroker = nullptr;
        error = FT_Stroker_New(normal_glyph->library, &stroker);
        if (error)
        {
            DV_LOG->writef_error("{} | FT_Stroker_New(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем обводчик в конце функции
        ScopeGuard done_stroker = [&stroker] { FT_Stroker_Done(stroker); };

        // Толщина должна быть в формате 26.6
        FT_Fixed thickness = static_cast<FT_Fixed>(settings.outline_thickness * 64);
        // Настраиваем обводчик
        FT_Stroker_Set(stroker, thickness, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

        // Создаём раздутый глиф из обычного, не уничтожая обычный
        FT_Glyph inflated_glyph = normal_glyph;
        FT_Glyph_StrokeBorder(&inflated_glyph, stroker, 0, 0); // Указатель inflated_glyph поменялся

        // Автоматически уничтожаем глиф в конце функции
        ScopeGuard done_inflated_glyph = [&inflated_glyph] { FT_Done_Glyph(inflated_glyph); };

        // Рендерим раздутый глиф
        FT_Render_Mode render_mode = settings.anti_aliasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
        error = FT_Glyph_To_Bitmap(&inflated_glyph, render_mode, nullptr, 1);
        if (error)
        {
            DV_LOG->writef_error("{} | Inflated FT_Glyph_To_Bitmap(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }

        FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(inflated_glyph); // Это указатель
        // Преобразуем отрендеренный глиф в grayscale Image
        image = to_image(bitmap_glyph->bitmap);
        i32 inflated_glyph_left = bitmap_glyph->left;
        i32 inflated_glyph_top = bitmap_glyph->top;

        // См. коммент SFGlyph::offset
        sf_glyph.offset.x = round_to_pixels(slot->metrics.horiBearingX);
        sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - slot->metrics.horiBearingY);
        // См. примечание https://freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#ft_glyph_strokeborder
        sf_glyph.advance_x = round_to_i32(to_f32(slot->metrics.horiAdvance) + settings.outline_thickness * 2);

        // Размываем grayscale глиф. Функция модифицирует sf_glyph.offset
        blur(settings.outline_blur_radius);
        // Окрашиваем
        image = image.to_rgba(settings.outline_color);

        // Если внутренний глиф не нужен, то на этом и заканчиваем
        if (!settings.render_inner)
            return;

        // ======================= Рендерим внутренний глиф обычным способом =======================

        // Рендерим
        error = FT_Glyph_To_Bitmap(&normal_glyph, render_mode, nullptr, 1);
        if (error)
        {
            DV_LOG->writef_error("{} | Inner FT_Glyph_To_Bitmap(...) error | {}", DV_FUNCSIG, error);
            error_image();
            return;
        }

        bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(normal_glyph);
        // Преобразуем отрендеренный глиф в grayscale Image
        Image inner_image = to_image(bitmap_glyph->bitmap);
        i32 inner_glyph_left = bitmap_glyph->left;
        i32 inner_glyph_top = bitmap_glyph->top;

        // ======================= Объединяем рендеры =======================

        // Смещение нормального изображения относительно раздутого.
        // Оно не всегда равно толщине обводки. Поэтому вычисляем так
        i32 delta_x = inner_glyph_left - inflated_glyph_left + settings.outline_blur_radius;
        i32 delta_y = inflated_glyph_top - inner_glyph_top + settings.outline_blur_radius;

        // Накладываем нормальный глиф на раздутый
        for (i32 y = 0; y < inner_image.size().y; ++y)
        {
            for (i32 x = 0; x < inner_image.size().x; ++x)
            {
                u32 back_color;
                memcpy(&back_color, image.pixel_ptr(x + delta_x, y + delta_y), 4);

                u32 front_color = settings.main_color;

                u32 mask = inner_image.pixel_ptr(x, y)[0];
                u32 result_r = get_r(front_color) * mask + get_r(back_color) * (0xFFu - mask);
                u32 result_g = get_g(front_color) * mask + get_g(back_color) * (0xFFu - mask);
                u32 result_b = get_b(front_color) * mask + get_b(back_color) * (0xFFu - mask);
                u32 result_a = get_a(front_color) * mask + get_a(back_color) * (0xFFu - mask);
                u32 result_color = to_rgba(static_cast<u8>(result_r / 0xFF), static_cast<u8>(result_g / 0xFF), static_cast<u8>(result_b / 0xFF), static_cast<u8>(result_a / 0xFF));
                memcpy(image.pixel_ptr(x + delta_x, y + delta_y), &result_color, 4);
            }
        }
    }

    // Запрещаем копирование
    RenderedGlyph(const RenderedGlyph&) = delete;
    RenderedGlyph& operator=(const RenderedGlyph&) = delete;

    // Разрешаем перемещение
    RenderedGlyph(RenderedGlyph&&) noexcept = default;
    RenderedGlyph& operator=(RenderedGlyph&&) noexcept = default;
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
        r.w = rendered_glyph.image.width() + padding * 2;
        r.h = rendered_glyph.image.height() + padding * 2;
        rects_.push_back(r);

        rendered_glyphs_.push_back(std::move(rendered_glyph));
    }

    // Возвращает набор атласов. Меняет исходные данные
    vector<shared_ptr<Image>> pack(const ivec2 texture_size)
    {
#ifndef NDEBUG
        // Проверяем, что метод не вызывается повторно
        assert(!packed);
        packed_ = true;
#endif

        assert(rendered_glyphs_.size() > 0);

        vector<shared_ptr<Image>> ret;

        stbrp_context pack_context;
        const i32 num_nodes = texture_size.x;
        vector<stbrp_node> nodes(num_nodes);

        while (rects_.size())
        {
            // Откладываем создание атласа, пока не будет найдет первый непустой глиф,
            // так как не знаем число цветовых каналов
            shared_ptr<Image> current_atlas = nullptr;

            stbrp_init_target(&pack_context, texture_size.x, texture_size.y, nodes.data(), num_nodes);
            stbrp_pack_rects(&pack_context, rects_.data(), (i32)rects_.size());

            for (size_t i = 0; i < rects_.size();)
            {
                stbrp_rect& rect = rects_[i];

                if (rect.was_packed)
                {
                    RenderedGlyph& rg = rendered_glyphs_[rect.id];

                    if (!rg.image.empty())
                    {
                        if (!current_atlas)
                            current_atlas = make_shared<Image>(texture_size, rg.image.num_components());

                        current_atlas->paste(rg.image, ivec2(rect.x + padding, rect.y + padding));
                    }

                    rg.sf_glyph.atlas_index = (i32)ret.size();
                    rg.sf_glyph.rect.pos = ivec2(rect.x, rect.y) + padding;
                    rg.sf_glyph.rect.size = ivec2(rect.w, rect.h) - padding * 2;
                    assert(rg.sf_glyph.rect.size.x == rg.image.size().x);

                    // Удаляем упакованный прямоугольник из списка, путём перемещения в конец
                    rect = std::move(rects_.back());
                    rects_.pop_back();
                }
                else
                {
                    ++i;
                }
            }

            assert(current_atlas);
            ret.push_back(current_atlas);
        }

        return ret;
    }
};

// FT_Face - это указатель.
// addend - это число, которое нужно прибавить к высоте.
// Такая модификация нужна для контурных шрифтов и шрифтов с обводкой:
// https://freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#ft_glyph_stroke
// https://freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#ft_glyph_strokeborder
static i32 calc_line_height(const FT_Face face, const f32 addend = 0.f)
{
    // TODO:
    // face->size->metrics.height - это расстояние между базовыми линиями.
    // Возможно нужно использовать `ascender - descender`.
    // Смотри коммент к FT_FaceRec_ в freetype.h
    FT_Pos height = face->size->metrics.height;

    if (addend == 0.f)
        return round_to_pixels(height);
    else
        return round_to_i32(to_f32(height) + addend); // Складываем перед округлением
}

// Код, одинаковый для всех конструкторов
template <typename T>
void render_glyphs(const FreeTypeFace& face,
                       const T& settings,
                       unordered_map<c32, SFGlyph>& out_glyphs,
                       vector<shared_ptr<Texture>>& out_textures)
{
    // Упаковщик глифов в аталасы
    GlyphPacker glyph_packer(face.get()->num_glyphs);

    FT_UInt glyph_index;
    // Получаем первый глиф
    FT_ULong char_code = FT_Get_First_Char(face.get(), &glyph_index);

    // Цикл по всем глифам шрифта
    while (glyph_index != 0)
    {
        // Рендерим глиф
        RenderedGlyph rendered_glyph(face.get(), settings, glyph_index, char_code);

        // Добавляем глиф в список для будущей упаковки в атласы
        glyph_packer.add(std::move(rendered_glyph));

        // Переходим к следующему глифу
        char_code = FT_Get_Next_Char(face.get(), char_code, &glyph_index);
    }

    // Упаковываем глифы в атласы
    vector<shared_ptr<Image>> atlases = glyph_packer.pack(settings.texture_size);

    // Заполняем таблицу code point → область в аталасе
    for (const RenderedGlyph& rendered_glyph : glyph_packer.rendered_glyphs())
        out_glyphs[rendered_glyph.code_point] = rendered_glyph.sf_glyph;

    for (shared_ptr<Image> atlas : atlases)
    {
        // Отправляем атлас в память GPU
        shared_ptr<Texture> texture = make_shared<Texture>(atlas, true);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Добавляем текстуру в кэш
        DV_TEXTURE_CACHE->add(texture);

        // Добавляем текстуру в список атласов спрайтового шрифта
        out_textures.push_back(texture);
    }
}

SpriteFont::SpriteFont(const SFSettingsSimple& settings, i64* generation_time_ms)
{
    // Засекаем время начала генерации
    const i64 begin_time_ms = get_ticks_ms();

    // Загружаем векторный шрифт из файла
    FreeTypeFace face(settings);

    // Рендерим
    render_glyphs(face, settings, glyphs_, textures_);

    // Определяем высоту строки
    line_height_ = calc_line_height(face.get());

    // Необязательная информация о настройках генератора
    src_info_ = "file: " + get_file_name(settings.src_path)
              + ", style: simple"
              + ", height: " + to_string(settings.height);

    // Вычисляем время генерации
    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_debug("{} | {} | Generated in {} ms", DV_FUNCSIG, settings.src_path, duration_ms);
}

SpriteFont::SpriteFont(const SFSettingsContour& settings, i64* generation_time_ms)
{
    // Засекаем время начала генерации
    const i64 begin_time_ms = get_ticks_ms();

    // Загружаем векторный шрифт из файла
    FreeTypeFace face(settings);

    // Рендерим
    render_glyphs(face, settings, glyphs_, textures_);

    // Определяем высоту строки
    line_height_ = calc_line_height(face.get(), settings.thickness);

    // Необязательная информация о настройках генератора
    src_info_ = "file: " + get_file_name(settings.src_path)
              + ", style: contour"
              + ", height: " + to_string(settings.height);

    // Вычисляем время генерации
    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_debug("{} | {} | Generated in {} ms", DV_FUNCSIG, settings.src_path, duration_ms);
}

SpriteFont::SpriteFont(const SFSettingsOutlined& settings, i64* generation_time_ms)
{
    // Засекаем время начала генерации
    const i64 begin_time_ms = get_ticks_ms();

    // Загружаем векторный шрифт из файла
    FreeTypeFace face(settings);

    // Рендерим
    render_glyphs(face, settings, glyphs_, textures_);

    // Определяем высоту строки
    line_height_ = calc_line_height(face.get(), settings.outline_thickness * 2);

    // Необязательная информация о настройках генератора
    src_info_ = "file: " + get_file_name(settings.src_path)
              + ", style: outlined"
              + ", height: " + to_string(settings.height);

    // Вычисляем время генерации
    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    DV_LOG->writef_debug("{} | {} | Generated in {} ms", DV_FUNCSIG, settings.src_path, duration_ms);
}

} // namespace dviglo
