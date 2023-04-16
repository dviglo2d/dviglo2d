// Copyright (c) the Dviglo project
// License: MIT

#include "sprite_font.hpp"

#include "freetype.hpp"

#include "../fs/file.hpp"
#include "../fs/log.hpp"
#include "../fs/path.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../gl_utils/gl_utils.hpp"

#include <pugixml.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#include <chrono>
#include <memory>

using namespace glm;
using namespace pugi;
using namespace std;


namespace dviglo
{

SpriteFont::SpriteFont(const StrUtf8& file_path)
{
    xml_document doc;
    xml_parse_result result = doc.load_file(file_path.c_str());
    if (!result)
    {
        DV_LOG->writef_error("SpriteFont::SpriteFont(\"{}\") | !result", file_path);
        return;
    }

    xml_node root_node = doc.first_child();
    if (root_node.name() != string("font"))
    {
        DV_LOG->writef_error("SpriteFont::SpriteFont(\"{}\") | root_node.name() != string(\"font\")", file_path);
        return;
    }

    xml_node pages_node = root_node.child("pages");
    if (!pages_node)
    {
        DV_LOG->writef_error("SpriteFont::SpriteFont(\"{}\") | !pages_node", file_path);
        return;
    }

    xml_node info_node = root_node.child("info");
    if (info_node)
    {
        face_ = info_node.attribute("face").as_string();
        size_ = stoi(info_node.attribute("size").value());
    }

    xml_node common_node = root_node.child("common");
    line_height_ = common_node.attribute("lineHeight").as_int(); // TODO: Переименовать в line_height
    i32 pages = common_node.attribute("pages").as_int();
    textures_.reserve(pages);

    string directory_path = get_parent(file_path);

    xml_node page_node = pages_node.child("page");
    for (i32 i = 0; i < pages; ++i)
    {
        if (!page_node)
        {
            DV_LOG->writef_error("SpriteFont::SpriteFont(\"{}\") | !page_node", file_path);
            return;
        }

        string image_file_name = page_node.attribute("file").as_string();
        string image_file_path = directory_path + image_file_name;
        textures_.push_back(DV_TEXTURE_CACHE->get(image_file_path));

        page_node = page_node.next_sibling();
    }

    for (xml_node char_node : root_node.child("chars"))
    {
        c32 id = char_node.attribute("id").as_uint(); // TODO: Переименовать в code_point
        Glyph glyph;
        glyph.rect.pos.x = char_node.attribute("x").as_int();
        glyph.rect.pos.y = char_node.attribute("y").as_int();
        glyph.rect.size.x = char_node.attribute("width").as_int();
        glyph.rect.size.y = char_node.attribute("height").as_int();
        glyph.offset = ivec2(char_node.attribute("xoffset").as_int(), char_node.attribute("yoffset").as_int()); // TODO: Переименовать в offset_x
        glyph.advance_x = char_node.attribute("xadvance").as_int();
        glyph.page = char_node.attribute("page").as_int();
        glyphs_[id] = glyph;
    }

    // TODO кернинг не загружается
}

void SpriteFont::save(const StrUtf8& file_path)
{
    // Проверяем, что текстуры содержат ссылки на изображения
    for (shared_ptr<Texture>& texture : textures_)
    {
        if (!texture->image())
        {
            DV_LOG->writef_error("SpriteFont::save(\"{}\") | !texture->image()", file_path);
            return;
        }
    }

    StrUtf8 dir_path, file_name, ext;
    split_path(file_path, &dir_path, &file_name, &ext);

    if (!ext.empty() && ext != "fnt")
    {
        DV_LOG->writef_error("SpriteFont::save(\"{}\") | ext != \"fnt\"", file_path);
        return;
    }

    // Сохраняем текстуры
    for (size_t i = 0; i < textures_.size(); ++i)
        textures_[i]->image()->save_png(dir_path + file_name + "_" + to_string(i) + ".png");

    xml_document doc;
    xml_node root_node = doc.append_child("font");

    xml_node info_node = root_node.append_child("info");
    info_node.append_attribute("face") = face_.c_str();
    info_node.append_attribute("size") = size_;

    xml_node chars_node = root_node.append_child("chars");
    chars_node.append_attribute("count") = glyphs_.size();

    // Копируем ключи в вектор
    vector<c32> code_points;
    code_points.reserve(glyphs_.size());
    for (const auto& pair : glyphs_)
        code_points.push_back(pair.first);

    // Сортируем ключи
    sort(code_points.begin(), code_points.end());

    for (c32 code_point : code_points)
    {
        Glyph& glyph = glyphs_[code_point];

        xml_node char_node = chars_node.append_child("char");
        char_node.append_attribute("id") = code_point; // TODO: Переименовать в code_point
        char_node.append_attribute("x") = glyph.rect.pos.x;
        char_node.append_attribute("y") = glyph.rect.pos.y;
        char_node.append_attribute("width") = glyph.rect.size.x;
        char_node.append_attribute("height") = glyph.rect.size.y;
        char_node.append_attribute("xoffset") = glyph.offset.x; // TODO: Переименовать в offset_x
        char_node.append_attribute("yoffset") = glyph.offset.y;
        char_node.append_attribute("xadvance") = glyph.advance_x;
        char_node.append_attribute("page") = glyph.page;
    }

    xml_node common_node = root_node.append_child("common");
    common_node.append_attribute("lineHeight") = line_height_; // TODO: Переименовать в line_height
    common_node.append_attribute("pages") = textures_.size();

    xml_node pages_node = root_node.append_child("pages");
    for (size_t i = 0; i < textures_.size(); ++i)
    {
        xml_node page_node = pages_node.append_child("page");
        page_node.append_attribute("id") = i;
        page_node.append_attribute("file") = (file_name + "_" + to_string(i) + ".png").c_str();
    }

    doc.save_file(file_path.c_str(), "    ");
}

// Шрифт
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
            DV_LOG->write_error("FreeTypeFace::FreeTypeFace(): data.empty()");
            return;
        }

        FT_Error error = FT_New_Memory_Face(DV_FREETYPE->library(), (const FT_Byte*)data.data(), (FT_Long)data.size(), 0, &face_);
        if (error)
        {
            DV_LOG->writef_error("FreeTypeFace::FreeTypeFace(): FT_New_Memory_Face() error {}", error);
            return;
        }

        error = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
        if (error)
        {
            DV_LOG->writef_error("FreeTypeFace::FreeTypeFace(): FT_Select_Charmap() error {}", error);
            return;
        }

        // Шрифт может содержать несколько начертаний, но используется только первое
        if (face_->num_faces != 1)
            DV_LOG->writef_warning("FreeTypeFace::FreeTypeFace(): face_->num_faces != 1 | {}", face_->num_faces);

        // Реальная высота текста отличается от запрошенной
        error = FT_Set_Pixel_Sizes(face_, 0, font_settings.height);
        if (error)
        {
            DV_LOG->writef_error("FreeTypeFace::FreeTypeFace(): FT_Set_Pixel_Sizes() error {}", error);
            return;
        }
    }

    ~FreeTypeFace()
    {
        if (face_)
        {
            FT_Error error = FT_Done_Face(face_);
            if (error)
                DV_LOG->writef_error("FreeTypeFace::~FreeTypeFace(): error {}", error);
        }
    }

    // Запрещаем копирование
    FreeTypeFace(const FreeTypeFace&) = delete;
    FreeTypeFace& operator=(const FreeTypeFace&) = delete;

    FT_Face get() const { return face_; }
};

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

static i32 round_to_pixels(FT_Pos value)
{
    // В библиотеке FreeType пиксель равен 64 условным единицам.
    // value >> 6 - это деление нацело на 64 (0b1000000 становится 1).
    // value & 0x3f - остаток от деления на 64 (0b1000111 & 0b0111111 равно 0b000111)
    // Значит, если остаток от деления >= полпикселя, то добавляем пиксель.
    return (i32)(value >> 6) + (((value & 0x3f) >= 32) ? 1 : 0);
}

RenderedGlyph render_glyph_simpe(FT_Face face, const SFSettingsSimple& font_settings)
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

    if (font_settings.blur_radius > 0)
    {

        unique_ptr<Image> new_image = make_unique<Image>(ret.image->size() + font_settings.blur_radius * 2,
                                                         ret.image->num_components());

        // Вставляем в центр расширенного изображения
        new_image->paste(*ret.image, ivec2(font_settings.blur_radius, font_settings.blur_radius));

        // TODO: Изображение может быть нулевого размера
        new_image->blur_triangle(font_settings.blur_radius);

        ret.image = std::move(new_image);

        // Так как размытые глифы предназначены для создания теней, то их центры должны
        // совпадать с центрами неразмытых глифов.
        ret.offset -= font_settings.blur_radius;
    }

    return ret;
}

SpriteFont::SpriteFont(const SFSettingsSimple& settings)
{
    auto begin_time = chrono::high_resolution_clock::now();

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
        RenderedGlyph rendered_glyph = render_glyph_simpe(face.get(), settings);
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

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = end_time - begin_time;
    auto duration_ms = chrono::duration_cast<chrono::milliseconds>(duration).count();

    DV_LOG->writef_info("SpriteFont::SpriteFont(const FontSettingsSimple&) | {} | Generated im {} ms", settings.src_path, duration_ms);
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
    FT_Stroker_Set(stroker, font_settings.thickness * 64 / 2, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
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
    ret.x_advance += font_settings.thickness;
    // Конец

    if (font_settings.blur_radius > 0)
    {

        unique_ptr<Image> new_image = make_unique<Image>(ret.image->size() + font_settings.blur_radius * 2,
            ret.image->num_components());

        // Вставляем в центр расширенного изображения
        new_image->paste(*ret.image, ivec2(font_settings.blur_radius, font_settings.blur_radius));

        // TODO: Изображение может быть нулевого размера
        new_image->blur_triangle(font_settings.blur_radius);

        ret.image = std::move(new_image);

        // Так как размытые глифы предназначены для создания теней, то их центры должны
        // совпадать с центрами неразмытых глифов.
        ret.offset -= font_settings.blur_radius;
    }

    return ret;
}

SpriteFont::SpriteFont(const SFSettingsContour& settings)
{
    auto begin_time = chrono::high_resolution_clock::now();

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
    line_height_ += settings.thickness;
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

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = end_time - begin_time;
    auto duration_ms = chrono::duration_cast<chrono::milliseconds>(duration).count();

    DV_LOG->writef_info("SpriteFont::SpriteFont(const FontSettingsSimple&) | {} | Generated im {} ms", settings.src_path, duration_ms);
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
    ret.x_advance += settings.outline_thickness * 2;


    // Используем манипулятор просто как место временного хранения.
    Image normalGlyph(to_image(bitmapGlyph->bitmap));

    int normalGlyphLeft = bitmapGlyph->left;
    int normalGlyphTop = bitmapGlyph->top;
    FT_Done_Glyph(glyph);

    // Рендерим раздутый глиф.
    FT_Get_Glyph(face->glyph, &glyph);
    FT_Stroker stroker;
    FT_Stroker_New(glyph->library, &stroker);
    FT_Stroker_Set(stroker, settings.outline_thickness * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
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
        // Переделать
        inflatedGlyph.blur_triangle(settings.outline_blur_radius);
        deltaX += settings.outline_blur_radius;
        deltaY += settings.outline_blur_radius;
        ret.offset.x -= settings.outline_blur_radius;
        ret.offset.y -= settings.outline_blur_radius;
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

SpriteFont::SpriteFont(const SFSettingsOutlined& settings)
{
    auto begin_time = chrono::high_resolution_clock::now();

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
    line_height_ += settings.outline_thickness * 2;
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

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = end_time - begin_time;
    auto duration_ms = chrono::duration_cast<chrono::milliseconds>(duration).count();

    DV_LOG->writef_info("SpriteFont::SpriteFont(const FontSettingsSimple&) | {} | Generated im {} ms", settings.src_path, duration_ms);
}

} // namespace dviglo
