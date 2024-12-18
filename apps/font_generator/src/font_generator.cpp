#include "font_generator.hpp"

#include <stb_rect_pack.h>


GeneratedFont generate_font_simple(const FreeTypeFace& face, const FontSettings& font_settings)
{
    GeneratedFont ret;

    //FT_UInt glyphIndex_ = FT_Get_Char_Index(face.get(), 0x2DE0);

    uint32_t glyph_index = FT_Get_Char_Index(face.get(), 'A');

    //FT_Set_Pixel_Sizes(face, 0, 48);

    
    /*FT_Load_Char(face.get(), 'A', FT_LOAD_DEFAULT);

    FT_UInt glyphIndex_ = 0;

    FT_ULong charCode = FT_Get_First_Char(face.get(), &glyphIndex_);

    for (i32 j = 0; j < 127; ++j)
        charCode = FT_Get_Next_Char(face.get(), charCode, &glyphIndex_);*/

    FT_Error error1 = FT_Load_Glyph(face.get(), glyph_index, FT_LOAD_TARGET_MONO);

    //FT_Error error1 = FT_Load_Char(face.get(), 'A', FT_LOAD_DEFAULT);

    if (error1)
    {
        DV_LOG->write_error(format("render_glyph_simpe(): FT_Load_Glyph() error | {}", error1));
        return ret;
    }

    RenderedGlyph rendered_glyph = render_glyph_simpe(face.get(), font_settings);

    ret.pages.push_back(rendered_glyph.grayscale_image->to_image());

    return ret;
}
