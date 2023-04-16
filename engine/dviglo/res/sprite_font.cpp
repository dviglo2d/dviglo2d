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
        glyph.advance_x = char_node.attribute("advance_x").as_int();
        glyph.page = char_node.attribute("page").as_int();
        glyphs_[id] = glyph;
    }

    // TODO кернинг не загружается
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
        const Glyph& glyph = glyphs_.at(code_point);

        xml_node char_node = chars_node.append_child("char");
        char_node.append_attribute("id") = code_point; // TODO: Переименовать в code_point
        char_node.append_attribute("x") = glyph.rect.pos.x;
        char_node.append_attribute("y") = glyph.rect.pos.y;
        char_node.append_attribute("width") = glyph.rect.size.x;
        char_node.append_attribute("height") = glyph.rect.size.y;
        char_node.append_attribute("xoffset") = glyph.offset.x; // TODO: Переименовать в offset_x
        char_node.append_attribute("yoffset") = glyph.offset.y;
        char_node.append_attribute("advance_x") = glyph.advance_x;
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

} // namespace dviglo
