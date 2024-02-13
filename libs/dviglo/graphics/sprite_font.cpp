// Copyright (c) the Dviglo project
// License: MIT

#include "sprite_font.hpp"

#include "../fs/log.hpp"
#include "../fs/path.hpp"
#include "../gl_utils/texture_cache.hpp"

#include <pugixml.hpp>

#include <format>

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
        DV_LOG->write_error(format("Font::Font(\"{}\") | !result", file_path));
        return;
    }

    xml_node root_node = doc.first_child();
    if (root_node.name() != string("font"))
    {
        DV_LOG->write_error(format("Font::Font(\"{}\") | root_node.name() != string(\"font\")", file_path));
        return;
    }

    xml_node pages_node = root_node.child("pages");
    if (!pages_node)
    {
        DV_LOG->write_error(format("Font::Font(\"{}\") | !pages_node", file_path));
        return;
    }

    xml_node info_node = root_node.child("info");
    if (info_node)
        size_ = stoi(info_node.attribute("size").value());

    xml_node common_node = root_node.child("common");
    line_height_ = common_node.attribute("lineHeight").as_int();
    i32 pages = common_node.attribute("pages").as_int();
    textures_.reserve(pages);

    string directory_path = get_parent(file_path);

    xml_node page_node = pages_node.child("page");
    for (i32 i = 0; i < pages; ++i)
    {
        if (!page_node)
        {
            DV_LOG->write_error(format("Font::Font(\"{}\") | !page_node", file_path));
            return;
        }

        string image_file_name = page_node.attribute("file").as_string();
        string image_file_path = directory_path + image_file_name;
        textures_.push_back(DV_TEXTURE_CACHE->get(image_file_path));

        page_node = page_node.next_sibling();
    }

    for (xml_node char_node : root_node.child("chars"))
    {
        u32 id = char_node.attribute("id").as_uint();
        Glyph glyph;
        glyph.x = (i16)char_node.attribute("x").as_uint();
        glyph.y = (i16)char_node.attribute("y").as_uint();
        glyph.width = (i16)char_node.attribute("width").as_uint();
        glyph.height = (i16)char_node.attribute("height").as_uint();
        glyph.offset_x = (i16)char_node.attribute("xoffset").as_uint();
        glyph.offset_y = (i16)char_node.attribute("yoffset").as_uint();
        glyph.advance_x = (i16)char_node.attribute("xadvance").as_uint();
        glyph.page = (i16)char_node.attribute("page").as_uint();
        glyphs_[id] = glyph;
    }

    // TODO кернинг не загружается
}

} // namespace dviglo
