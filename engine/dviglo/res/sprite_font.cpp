// Copyright (c) the Dviglo project
// License: MIT

#include "sprite_font.hpp"

#include "../fs/log.hpp"
#include "../fs/path.hpp"
#include "../gl_utils/texture_cache.hpp"

#include <pugixml.hpp>

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

    line_height_ = root_node.attribute("line_height").as_int();
    src_info_ = root_node.attribute("src_info").as_string();

    xml_node atlases_node = root_node.child("atlases");
    if (!atlases_node)
    {
        DV_LOG->writef_error("SpriteFont::SpriteFont(\"{}\") | !atlases_node", file_path);
        return;
    }

    i32 num_atlases = atlases_node.attribute("count").as_int();
    textures_.reserve(num_atlases);

    string directory_path = get_parent(file_path);

    xml_node atlas_node = atlases_node.child("atlas");
    for (i32 i = 0; i < num_atlases; ++i)
    {
        if (!atlas_node)
        {
            DV_LOG->writef_error("SpriteFont::SpriteFont(\"{}\") | !atlas_node", file_path);
            return;
        }

        // TODO: Индекс аталаса не считывается
        string image_file_name = atlas_node.attribute("file").as_string();
        string image_file_path = directory_path + image_file_name;
        textures_.push_back(DV_TEXTURE_CACHE->get(image_file_path));

        atlas_node = atlas_node.next_sibling();
    }

    // TODO: сравнить число загруженных атласов и atlas count

    // TODO: Считывать count у chars и сравнить с числом записей

    for (xml_node glyph_node : root_node.child("glyphs"))
    {
        c32 code_point = glyph_node.attribute("code_point").as_uint();
        SFGlyph glyph;
        glyph.rect.pos.x = glyph_node.attribute("x").as_int();
        glyph.rect.pos.y = glyph_node.attribute("y").as_int();
        glyph.rect.size.x = glyph_node.attribute("width").as_int();
        glyph.rect.size.y = glyph_node.attribute("height").as_int();
        glyph.offset = ivec2(glyph_node.attribute("offset_x").as_int(), glyph_node.attribute("offset_y").as_int());
        glyph.advance_x = glyph_node.attribute("advance_x").as_int();
        glyph.atlas_index = glyph_node.attribute("atlas_index").as_int();
        glyphs_[code_point] = glyph;
    }

    // TODO кернинг не загружается
}

const SFGlyph& SpriteFont::get_glyph(c32 code_point)
{
    auto it = glyphs_.find(code_point);

    if (it == glyphs_.end())
        it = glyphs_.find('?'); // Вопрос всегда должен быть в шрифте

    assert(it != glyphs_.end());

    return it->second;
}

void SpriteFont::save(const StrUtf8& file_path) const
{
    // Проверяем, что текстуры содержат ссылки на изображения
    for (const shared_ptr<Texture>& texture : textures_)
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
    root_node.append_attribute("line_height") = line_height_;
    root_node.append_attribute("src_info") = src_info_.c_str();

    xml_node atlases_node = root_node.append_child("atlases");
    atlases_node.append_attribute("count") = textures_.size();
    for (size_t i = 0; i < textures_.size(); ++i)
    {
        xml_node atlas_node = atlases_node.append_child("atlas");
        atlas_node.append_attribute("index") = i;
        atlas_node.append_attribute("file") = (file_name + "_" + to_string(i) + ".png").c_str();
    }

    xml_node glyphs_node = root_node.append_child("glyphs");
    glyphs_node.append_attribute("count") = glyphs_.size();

    // Копируем ключи в вектор
    vector<c32> code_points;
    code_points.reserve(glyphs_.size());
    for (const auto& pair : glyphs_)
        code_points.push_back(pair.first);

    // Сортируем ключи
    sort(code_points.begin(), code_points.end());

    for (c32 code_point : code_points)
    {
        const SFGlyph& glyph = glyphs_.at(code_point);

        xml_node glyph_node = glyphs_node.append_child("glyph");
        glyph_node.append_attribute("code_point") = code_point;
        glyph_node.append_attribute("x") = glyph.rect.pos.x;
        glyph_node.append_attribute("y") = glyph.rect.pos.y;
        glyph_node.append_attribute("width") = glyph.rect.size.x;
        glyph_node.append_attribute("height") = glyph.rect.size.y;
        glyph_node.append_attribute("offset_x") = glyph.offset.x;
        glyph_node.append_attribute("offset_y") = glyph.offset.y;
        glyph_node.append_attribute("advance_x") = glyph.advance_x;
        glyph_node.append_attribute("atlas_index") = glyph.atlas_index;
    }

    doc.save_file(file_path.c_str(), "    ", format_no_declaration | format_indent);
}

} // namespace dviglo
