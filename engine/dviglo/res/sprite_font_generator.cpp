// Copyright (c) the Dviglo project
// License: MIT

// Конструкторы SpriteFont, которые используют FreeType

#include "sprite_font.hpp"

#include "freetype.hpp"
#include "freetype_misc.hpp"

#include "../gl_utils/gl_utils.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../main/timer.hpp"

// Размываем на GPU, а не на CPU.
// Размывать глифы по одному нельзя, так как будет много draw call-ов.
// Поэтому сперва объединяем глифы в атласы, а потом размываем атласы целиком.
// У глифов с обводкой размывается только обводка, поэтому для них хранится два набора grayscale атласов (слоя):
// раздутые глифы + обычные глифы, которые будут наложены на раздутые.
// В финале слои окрашиваются и комбинируются в один набор атласов
#define DV_GPU_BLUR 1

#if DV_GPU_BLUR
    #include "../gl_utils/shader_cache.hpp"
#endif

#include <dv_file.hpp>
#include <dv_log.hpp>
#include <dv_math.hpp>
#include <dv_scope_guard.hpp>

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
                Log::writef_error("{} | FT_Done_Face(...) error {}", DV_FUNC_SIG, error);

            face_ = nullptr;
        }
    }

public:
    FreeTypeFace(const SFSettings& settings)
    {
        data = read_all_data(settings.src_path);
        if (data.empty())
        {
            Log::writef_error("{} | data.empty()", DV_FUNC_SIG);
            return;
        }

        // Шрифт может содержать несколько начертаний, но используется только первое
        FT_Error error = FT_New_Memory_Face(DV_FREETYPE->library(),
                                            reinterpret_cast<const FT_Byte*>(data.data()),
                                            static_cast<FT_Long>(data.size()),
                                            0, &face_);
        if (error)
        {
            Log::writef_error("{} | FT_New_Memory_Face(...) error {}", DV_FUNC_SIG, error);
            done();
            return;
        }

        if (face_->num_faces != 1)
            Log::writef_warning("{} | face_->num_faces != 1 | {}", DV_FUNC_SIG, face_->num_faces);

        error = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
        if (error)
        {
            Log::writef_error("{} | FT_Select_Charmap(...) error {}", DV_FUNC_SIG, error);
            done();
            return;
        }

        // Реальная высота текста чаще всего отличается от запрошенной
        error = FT_Set_Pixel_Sizes(face_, 0, settings.height);
        if (error)
        {
            Log::writef_error("{} | FT_Set_Pixel_Sizes(...) error {}", DV_FUNC_SIG, error);
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

    // Расширяет (если нужно) grayscale Image для будущего размытия.
    // Функция может модифицировать sf_glyph.offset, если передать указатель
    static void expand(Image& inout_image, const i32 blur_radius, ivec2* sf_glyph_offset)
    {
        assert(blur_radius >= 0);
        assert(inout_image.num_components() == 1 || inout_image.empty());

        if (!blur_radius)
            return;

        // Расширенное изображение
        Image new_image(inout_image.size() + blur_radius * 2, 1);

        // Вставляем исходное изображение в центр расширенного
        new_image.paste(inout_image, ivec2(blur_radius));

        inout_image = std::move(new_image);

        // Размытый текст предназначен для создания тени от неразмытого текста
        if (sf_glyph_offset)
            *sf_glyph_offset -= blur_radius;
    }

    void error_image()
    {
        assert(code_point);

        // TODO: Пурпурное изображение в случае ошибки
    }

public:
    // Результат рендеринга - одно grayscale изображение.
    // Только глиф с обводкой рендерится в 2 изображения: раздутый глиф + обычный глиф, который будет наложен на раздутый
    vector<Image> result;

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
            Log::writef_error("{} | FT_Load_Glyph(...) | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }

        const FT_GlyphSlot slot = face->glyph; // Это указатель

        // Импортируем grayscale глиф из слота
        result.push_back(to_image(slot->bitmap));

        sf_glyph.offset.x = round_to_pixels(slot->metrics.horiBearingX);
        sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - slot->metrics.horiBearingY);
        sf_glyph.advance_x = round_to_pixels(slot->metrics.horiAdvance);

        // Расширяем grayscale глиф (модифицирует sf_glyph.offset)
        expand(result[0], settings.blur_radius, &sf_glyph.offset);
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
            Log::writef_error("{} | FT_Load_Glyph(...) | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }

        const FT_GlyphSlot slot = face->glyph; // Это указатель

        // Импортируем глиф из слота
        FT_Glyph glyph = nullptr;
        error = FT_Get_Glyph(slot, &glyph);
        if (error)
        {
            Log::writef_error("{} | FT_Get_Glyph(...) error | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем глиф в конце функции. Передаём ссылку, так как значение меняется ниже
        DV_SCOPE_GUARD = [&glyph] { FT_Done_Glyph(glyph); };

        // Выделяем память для обводчика
        FT_Stroker stroker = nullptr;
        error = FT_Stroker_New(glyph->library, &stroker);
        if (error)
        {
            Log::writef_error("{} | FT_Stroker_New(...) error | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем обводчик в конце функции
        DV_SCOPE_GUARD = [&stroker] { FT_Stroker_Done(stroker); };

        // Радиус - половина толщины (делим на 2).
        // Радиус должен быть в формате 26.6 (умножаем на 64)
        FT_Fixed radius = static_cast<FT_Fixed>(settings.thickness * 32);
        // Настраиваем обводчик
        FT_Stroker_Set(stroker, radius, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

        // Заменяем глиф контуром глифа
        error = FT_Glyph_Stroke(&glyph, stroker, 1);
        if (error)
        {
            Log::writef_error("{} | FT_Glyph_Stroke(...) error | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }

        // Заменяем глиф битмапом (рендерим)
        FT_Render_Mode render_mode = settings.anti_aliasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
        error = FT_Glyph_To_Bitmap(&glyph, render_mode, nullptr, 1);
        if (error)
        {
            Log::writef_error("{} | FT_Glyph_To_Bitmap(...) error | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }

        // Преобразуем отрендеренный глиф в grayscale Image
        FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph); // Это указатель
        result.push_back(to_image(bitmap_glyph->bitmap));

        // См. коммент SFGlyph::offset
        sf_glyph.offset.x = round_to_pixels(slot->metrics.horiBearingX);
        sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - slot->metrics.horiBearingY);

        // См. примечание https://freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#ft_glyph_stroke
        sf_glyph.advance_x = round_to_i32(to_f32(slot->metrics.horiAdvance) + settings.thickness);

        // Расширяем grayscale глиф. Функция модифицирует sf_glyph.offset
        expand(result[0], settings.blur_radius, &sf_glyph.offset);
    }

    // Рендерит глиф с обводкой
    RenderedGlyph(const FT_Face face, const SFSettingsOutlined& settings,
                  const FT_UInt glyph_index, const FT_ULong char_code)
    {
        assert(settings.outline_blur_radius >= 0);

        if (settings.render_inner)
            result.reserve(2);

        code_point = static_cast<c32>(char_code);

        FT_Int32 load_flags = FT_LOAD_NO_BITMAP;
        // Алгоритм хинтига
        load_flags |= settings.anti_aliasing ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO;
        // Загружаем глиф в слот
        FT_Error error = FT_Load_Glyph(face, glyph_index, load_flags);
        if (error)
        {
            Log::writef_error("{} | FT_Load_Glyph(...) | {}", DV_FUNC_SIG, error);
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
            Log::writef_error("{} | FT_Get_Glyph(...) error | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем глиф в конце функции
        DV_SCOPE_GUARD = [&normal_glyph] { FT_Done_Glyph(normal_glyph); };

        // Выделяем память для обводчика
        FT_Stroker stroker = nullptr;
        error = FT_Stroker_New(normal_glyph->library, &stroker);
        if (error)
        {
            Log::writef_error("{} | FT_Stroker_New(...) error | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }
        // Автоматически уничтожаем обводчик в конце функции
        DV_SCOPE_GUARD = [&stroker] { FT_Stroker_Done(stroker); };

        // Толщина должна быть в формате 26.6
        FT_Fixed thickness = static_cast<FT_Fixed>(settings.outline_thickness * 64);
        // Настраиваем обводчик
        FT_Stroker_Set(stroker, thickness, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

        // Создаём раздутый глиф из обычного, не уничтожая обычный
        FT_Glyph inflated_glyph = normal_glyph;
        FT_Glyph_StrokeBorder(&inflated_glyph, stroker, 0, 0); // Указатель inflated_glyph поменялся

        // Автоматически уничтожаем глиф в конце функции
        DV_SCOPE_GUARD = [&inflated_glyph] { FT_Done_Glyph(inflated_glyph); };

        // Рендерим раздутый глиф
        FT_Render_Mode render_mode = settings.anti_aliasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
        error = FT_Glyph_To_Bitmap(&inflated_glyph, render_mode, nullptr, 1);
        if (error)
        {
            Log::writef_error("{} | Inflated FT_Glyph_To_Bitmap(...) error | {}", DV_FUNC_SIG, error);
            error_image();
            return;
        }

        FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(inflated_glyph); // Это указатель
        // Преобразуем отрендеренный глиф в grayscale Image
        result.push_back(to_image(bitmap_glyph->bitmap));
        i32 inflated_glyph_left = bitmap_glyph->left;
        i32 inflated_glyph_top = bitmap_glyph->top;

        // См. коммент SFGlyph::offset
        sf_glyph.offset.x = round_to_pixels(slot->metrics.horiBearingX);
        sf_glyph.offset.y = round_to_pixels(face->size->metrics.ascender - slot->metrics.horiBearingY);
        // См. примечание https://freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#ft_glyph_strokeborder
        sf_glyph.advance_x = round_to_i32(to_f32(slot->metrics.horiAdvance) + settings.outline_thickness * 2);

        // Расширяем grayscale глиф. Функция модифицирует sf_glyph.offset
        expand(result[0], settings.outline_blur_radius, &sf_glyph.offset);

        // Если внутренний глиф не нужен, то на этом и заканчиваем
        if (!settings.render_inner)
            return;

        // ======================= Рендерим внутренний глиф обычным способом =======================

        // Создаём второй слой такого же размера, как и первый
        result.emplace_back(result[0].size(), 1, 0x00000000, false); // Раздутый глиф может быть пустым

        if (bitmap_glyph->bitmap.width && bitmap_glyph->bitmap.rows) // Если глиф не пустой
        {
            // Рендерим
            error = FT_Glyph_To_Bitmap(&normal_glyph, render_mode, nullptr, 1);
            if (error)
            {
                Log::writef_error("{} | Inner FT_Glyph_To_Bitmap(...) error | {}", DV_FUNC_SIG, error);
                error_image();
                return;
            }

            bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(normal_glyph);

            i32 inner_glyph_left = bitmap_glyph->left;
            i32 inner_glyph_top = bitmap_glyph->top;

            // Смещение нормального изображения относительно раздутого.
            // Оно не всегда равно толщине обводки, поэтому вычисляем так
            i32 delta_x = inner_glyph_left - inflated_glyph_left + settings.outline_blur_radius;
            i32 delta_y = inflated_glyph_top - inner_glyph_top + settings.outline_blur_radius;

            // Преобразуем отрендеренный глиф в grayscale Image
            Image temp_image = to_image(bitmap_glyph->bitmap);

            for (i32 y = 0; y < temp_image.size().y; ++y)
            {
                for (i32 x = 0; x < temp_image.size().x; ++x)
                    memcpy(result[1].pixel_ptr(x + delta_x, y + delta_y), temp_image.pixel_ptr(x, y), 1);
            }
        }
    }

    // Запрещаем копирование
    RenderedGlyph(const RenderedGlyph&) = delete;
    RenderedGlyph& operator =(const RenderedGlyph&) = delete;

    // Разрешаем перемещение
    RenderedGlyph(RenderedGlyph&&) noexcept = default;
    RenderedGlyph& operator =(RenderedGlyph&&) noexcept = default;
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
    // Массив атласов
    using Layer = vector<shared_ptr<Image>>;

    // Результат упаковки.
    // Только глифы с обводкой рендерятся в два набора атласов: раздутые глифы + обычные глифы, которые будут наложены на раздутые.
    // У остальных стилей один слой
    vector<Layer> result;

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
        r.w = rendered_glyph.result[0].width() + padding * 2;
        r.h = rendered_glyph.result[0].height() + padding * 2;
        rects_.push_back(r);

        rendered_glyphs_.push_back(std::move(rendered_glyph));
    }

    // Возвращает набор атласов. Меняет исходные данные
    void pack(const ivec2 texture_size)
    {
#ifndef NDEBUG
        // Проверяем, что метод не вызывается повторно
        assert(!packed);
        packed_ = true;
#endif

        assert(rendered_glyphs_.size() > 0);

        const size_t num_layers = rendered_glyphs_[0].result.size();
        result.resize(num_layers);

        stbrp_context pack_context;
        const i32 num_nodes = texture_size.x;
        vector<stbrp_node> nodes(num_nodes);

        while (rects_.size())
        {
            // Создаём текущие атласы для каждого слоя
            for (size_t i = 0; i < num_layers; ++i)
                result[i].push_back(make_shared<Image>(texture_size, 1));

            stbrp_init_target(&pack_context, texture_size.x, texture_size.y, nodes.data(), num_nodes);
            stbrp_pack_rects(&pack_context, rects_.data(), (i32)rects_.size());

            for (size_t i = 0; i < rects_.size();)
            {
                stbrp_rect& rect = rects_[i];

                if (rect.was_packed)
                {
                    RenderedGlyph& rg = rendered_glyphs_[rect.id];

                    // Модифицируем текущий атлас каждого слоя
                    for (size_t layer_index = 0; layer_index < result.size(); ++layer_index)
                    {
                        shared_ptr<Image>& current_atlas = result[layer_index].back();
                        current_atlas->paste(rg.result[layer_index], ivec2(rect.x + padding, rect.y + padding));
                    }

                    rg.sf_glyph.atlas_index = (i32)(result[0].size() - 1);
                    rg.sf_glyph.rect.pos = ivec2(rect.x, rect.y) + padding;
                    rg.sf_glyph.rect.size = ivec2(rect.w, rect.h) - padding * 2;
                    assert(rg.sf_glyph.rect.size.x == rg.result[0].size().x);

                    // Удаляем упакованный прямоугольник из списка путём замены последним
                    rect = std::move(rects_.back());
                    rects_.pop_back();
                }
                else
                {
                    ++i;
                }
            }
        }
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

static void blur(Image& atlas, const i32 blur_radius)
{
    assert(blur_radius >= 0);
    assert(atlas.num_components() == 1 || atlas.empty());

#if DV_GPU_BLUR
    fs::path base_path = get_base_path();
    ShaderProgram* blur_shader = DV_SHADER_CACHE->get(base_path / "engine_data/shaders/texture_shader.vert", base_path / "engine_data/shaders/blur_triangle.frag");
    Texture tex(atlas);
    function<void()> f = [blur_shader, blur_radius]() { blur_shader->set("u_blur_radius", blur_radius); blur_shader->set("u_direction", ivec2(0, 1)); };
    tex.apply_shader(blur_shader, f);
    function<void()> f2 = [blur_shader, blur_radius]() { blur_shader->set("u_blur_radius", blur_radius); blur_shader->set("u_direction", ivec2(1, 0)); };
    tex.apply_shader(blur_shader, f2);
    tex.copy_to_cpu();
    atlas = std::move(*tex.image());
#else
    atlas.blur_triangle(blur_radius);
#endif
}

template <typename T>
SpriteFont::SpriteFont(const T& settings, i64* generation_time_ms)
{
    // Засекаем время начала генерации
    const i64 begin_time_ms = get_ticks_ms();

    // Загружаем векторный шрифт из файла
    FreeTypeFace face(settings);

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

    vector<shared_ptr<Image>> atlases;

    if constexpr (std::is_same_v<T, SFSettingsSimple> || std::is_same_v<T, SFSettingsContour>)
    {
        // Упаковываем глифы в атласы.
        // Размываем атласы после упаковки, а не отдельные глифы, чтобы уменьшить число draw call-ов
        glyph_packer.pack(settings.texture_size);

        assert(glyph_packer.result.size() == 1);
        atlases = std::move(glyph_packer.result[0]); // Всего один слой

        // Размываем атласы
        for (shared_ptr<Image>& atlas : atlases)
            blur(*atlas, settings.blur_radius);

        // Окрашиваем атласы
        for (shared_ptr<Image>& atlas : atlases)
            *atlas = std::move(atlas->to_rgba(settings.color));
    }
    else if constexpr (std::is_same_v<T, SFSettingsOutlined>)
    {
        // Упаковываем глифы в атласы.
        // Размываем атласы после упаковки, а не отдельные глифы, чтобы уменьшить число draw call-ов
        glyph_packer.pack(settings.texture_size);
        assert(glyph_packer.result.size() == 1 || glyph_packer.result.size() == 2);
        atlases = std::move(glyph_packer.result[0]); // Забираем раздутые глифы

        // Размываем атласы
        for (shared_ptr<Image>& atlas : atlases)
            blur(*atlas, settings.outline_blur_radius);

        // Окрашиваем и слепляем атласы
        for (size_t i = 0; i < atlases.size(); ++i)
        {
            Image combined_atlas = atlases[i]->to_rgba(settings.outline_color);

            if (settings.render_inner)
            {
                Image& inner_atlas = *glyph_packer.result[1][i];
                assert(combined_atlas.size() == inner_atlas.size());

                // Накладываем нормальные глифы на раздутые
                #pragma omp parallel for // Распараллеливание внешнего цикла с помощью OpenMP
                for (i32 y = 0; y < inner_atlas.size().y; ++y)
                {
                    for (i32 x = 0; x < inner_atlas.size().x; ++x)
                    {
                        u32 back_color;
                        memcpy(&back_color, combined_atlas.pixel_ptr(x, y), 4);

                        u32 front_color = settings.main_color;

                        u32 mask = (u32)inner_atlas.pixel_ptr(x, y)[0];
                        u32 result_r = get_r(front_color) * mask + get_r(back_color) * (0xFFu - mask);
                        u32 result_g = get_g(front_color) * mask + get_g(back_color) * (0xFFu - mask);
                        u32 result_b = get_b(front_color) * mask + get_b(back_color) * (0xFFu - mask);
                        u32 result_a = get_a(front_color) * mask + get_a(back_color) * (0xFFu - mask);
                        u32 result_color = to_rgba(static_cast<u8>(result_r / 0xFF), static_cast<u8>(result_g / 0xFF), static_cast<u8>(result_b / 0xFF), static_cast<u8>(result_a / 0xFF));
                        memcpy(combined_atlas.pixel_ptr(x, y), &result_color, 4);
                    }
                }
            }

            *atlases[i] = std::move(combined_atlas);
        }
    }
    else
    {
        static_assert(false);
    }

    // Заполняем таблицу code point → область в аталасе
    for (const RenderedGlyph& rendered_glyph : glyph_packer.rendered_glyphs())
        glyphs_[rendered_glyph.code_point] = rendered_glyph.sf_glyph;

    for (shared_ptr<Image> atlas : atlases)
    {
        // Отправляем атлас в память GPU
        shared_ptr<Texture> texture = make_shared<Texture>(atlas, true);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Добавляем текстуру в кэш
        DV_TEXTURE_CACHE->add(texture);

        // Добавляем текстуру в список атласов спрайтового шрифта
        textures_.push_back(texture);
    }

    if constexpr (std::is_same_v<T, SFSettingsSimple>)
    {
        // Определяем высоту строки
        line_height_ = calc_line_height(face.get());

        // Необязательная информация о настройках генератора
        src_info_ = "file: " + settings.src_path.filename().string()
                  + ", style: simple"
                  + ", height: " + to_string(settings.height);
    }
    else if constexpr (std::is_same_v<T, SFSettingsContour>)
    {
        // Определяем высоту строки
        line_height_ = calc_line_height(face.get(), settings.thickness);

        // Необязательная информация о настройках генератора
        src_info_ = "file: " + settings.src_path.filename().string()
                  + ", style: contour"
                  + ", height: " + to_string(settings.height);
    }
    else if constexpr (std::is_same_v<T, SFSettingsOutlined>)
    {
        // Определяем высоту строки
        line_height_ = calc_line_height(face.get(), settings.outline_thickness * 2);

        // Необязательная информация о настройках генератора
        src_info_ = "file: " + settings.src_path.filename().string()
                  + ", style: outlined"
                  + ", height: " + to_string(settings.height);
    }
    else
    {
        static_assert(false);
    }

    // Вычисляем время генерации
    i64 duration_ms = get_ticks_ms() - begin_time_ms;

    if (generation_time_ms)
        *generation_time_ms = duration_ms;

    Log::writef_debug("{} | {} | Generated in {} ms", DV_FUNC_SIG, settings.src_path, duration_ms);
}

// Явное инстанцирование конструкторов
template SpriteFont::SpriteFont(const SFSettingsSimple& settings, i64* generation_time_ms);
template SpriteFont::SpriteFont(const SFSettingsContour& settings, i64* generation_time_ms);
template SpriteFont::SpriteFont(const SFSettingsOutlined& settings, i64* generation_time_ms);

} // namespace dviglo
