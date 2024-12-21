#include "glyph_renderer.hpp"

#include <cassert>


RenderedGlyph render_glyph_simpe(FT_Face face, const FontSettings& font_settings)
{
    RenderedGlyph ret;

    assert(font_settings.blur_radius >= 0);
#if 0
    //FT_UInt glyphIndex_ = FT_Get_Char_Index(face.get(), 0x2DE0);

    uint32_t glyph_index = FT_Get_Char_Index(face, 41);

    //FT_Set_Pixel_Sizes(face, 0, 48);

    FT_Load_Char(face, 'A', FT_LOAD_DEFAULT);

    FT_UInt glyphIndex_ = 0;

    FT_ULong charCode = FT_Get_First_Char(face, &glyphIndex_);

    for (i32 j = 0; j < 127; ++j)
        charCode = FT_Get_Next_Char(face, charCode, &glyphIndex_);

    FT_Error error1 = FT_Load_Glyph(face, glyphIndex_, FT_LOAD_DEFAULT);

    //FT_Error error1 = FT_Load_Char(face.get(), 'A', FT_LOAD_DEFAULT);

    if (error1)
    {
        DV_LOG->write_error(format("render_glyph_simpe(): FT_Load_Glyph() error | {}", error1));
        return ret;
    }

#endif
    // Реднерим глиф
    FT_Glyph glyph;

    FT_Error error = FT_Get_Glyph(face->glyph, &glyph);

    if (error)
    {
        DV_LOG->write_error(format("render_glyph_simpe(): FT_Get_Glyph() error | {}", error));
        return ret;
    }

    //error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, nullptr, true);

//    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);

    if (error)
    {
        DV_LOG->write_error(format("render_glyph_simpe(): FT_Glyph_To_Bitmap() error | {}", error));
        FT_Done_Glyph(glyph);
        return ret;
    }

    //FT_Bitmap* ft_bitmap = &face->glyph->bitmap;


    FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    ret.grayscale_image = make_unique<GrayscaleImage>(bitmap_glyph);
    FT_Done_Glyph(glyph);

    return ret;

    /*
    int blurDistance = Abs(value2_);

    // Загружаем глиф.
    FT_Load_Glyph(face_, glyphIndex_, FT_LOAD_DEFAULT);

    // Метрики можно извлечь и без рендеринга.
    CalculateMetrics();

    // Реднерим глиф.
    FT_Glyph glyph;
    FT_Get_Glyph(face_->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    GlyphManipulator glyphManipulator(bitmapGlyph);
    FT_Done_Glyph(glyph);

    // Размываем, если нужно.
    if (blurDistance > 0)
    {
        glyphManipulator.Blur(blurDistance);

        // Так как размытые глифы предназначены для создания теней, то их центры должны
        // совпадать с центрами неразмытых глифов.
        xOffset_ -= blurDistance;
        yOffset_ -= blurDistance;
    }

    SharedPtr<Image> resultImage = ConvertToImage(glyphManipulator, mainColor_);
    PackGlyph(resultImage);
    */
}
