#include "glyph_renderer.hpp"

#include <cassert>


RenderedGlyph render_glyph_simpe(const FreeTypeFace& face, i32 blur_radius)
{
    RenderedGlyph ret;

    assert(blur_radius >= 0);

    // Реднерим глиф
    FT_Glyph glyph;
    FT_Get_Glyph(face.get()->glyph, &glyph);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
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
