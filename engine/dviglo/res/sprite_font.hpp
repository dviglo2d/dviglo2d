// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../gl_utils/texture.hpp"
#include "../math/rect.hpp"

#include <dv_fs.hpp>
#include <dv_string.hpp>
#include <limits>
#include <unordered_map>


namespace dviglo
{

struct SFGlyph
{
    // Область в текстурном атласе
    IntRect rect = IntRect::zero;

    // Смещение от origin (левый верхний угол) при рендеринге.
    // offset.x = horiBearingX
    // offset.y = ascender - horiBearingY
    // https://freetype.org/freetype2/docs/glyphs/glyphs-3.html
    // https://freetype.org/freetype2/docs/tutorial/step2.html
    glm::ivec2 offset{0, 0};

    // Расстояние между origin (левый верхний угол) текущего глифа и origin следующего глифа при рендеринге
    i32 advance_x = 0;

    // Индекс текстурного атласа
    i32 atlas_index = std::numeric_limits<i32>::max();
};


struct SFSettings
{
    fs::path src_path;
    i32 height; // В пикселях
    bool anti_aliasing;
    glm::ivec2 texture_size;

    SFSettings(const fs::path& src_path,
               i32 height = 20,
               bool anti_aliasing = true,
               glm::ivec2 texture_size = glm::ivec2(1024, 1024))
        : src_path(src_path)
        , height(height)
        , anti_aliasing(anti_aliasing)
        , texture_size(texture_size)
    {
    }
};

// Обычный шрифт
struct SFSettingsSimple : SFSettings
{
    i32 blur_radius;
    u32 color; // 0xAABBGGRR

    SFSettingsSimple(const fs::path& src_path,
                     i32 height = 20,
                     bool anti_aliasing = true,
                     i32 blur_radius = 0,
                     u32 color = 0xFFFFFFFF,
                     glm::ivec2 texture_size = glm::ivec2(1024, 1024))
        : SFSettings(src_path, height, anti_aliasing, texture_size)
        , blur_radius(blur_radius)
        , color(color)
    {
    }
};

// Только контур
struct SFSettingsContour : SFSettings
{
    f32 thickness;
    i32 blur_radius;
    u32 color; // 0xAABBGGRR

    SFSettingsContour(const fs::path& src_path,
                      i32 height = 20,
                      f32 thickness = 1.2f,
                      bool anti_aliasing = true,
                      i32 blur_radius = 0,
                      u32 color = 0xFFFFFFFF,
                      glm::ivec2 texture_size = glm::ivec2(1024, 1024))
        : SFSettings(src_path, height, anti_aliasing, texture_size)
        , thickness(thickness)
        , blur_radius(blur_radius)
        , color(color)
    {
    }
};

// Шрифт с обводкой
struct SFSettingsOutlined : SFSettings
{
    u32 main_color; // 0xAABBGGRR
    u32 outline_color; // 0xAABBGGRR
    f32 outline_thickness;
    i32 outline_blur_radius;
    bool render_inner; // Рендерить ли внутренний глиф

    SFSettingsOutlined(const fs::path& src_path,
                       i32 height = 20,
                       u32 main_color = 0xFFFFFFFF,
                       u32 outline_color = 0xFF000000,
                       f32 outline_thickness = 2,
                       i32 outline_blur_radius = 0,
                       bool render_inner = true,
                       bool anti_aliasing = true,
                       glm::ivec2 texture_size = glm::ivec2(1024, 1024))
        : SFSettings(src_path, height, anti_aliasing, texture_size)
        , main_color(main_color)
        , outline_color(outline_color)
        , outline_thickness(outline_thickness)
        , outline_blur_radius(outline_blur_radius)
        , render_inner(render_inner)
    {
    }
};

class SpriteFont
{
private:
    StrUtf8 src_info_; // Настройки генератора (просто информация, можно не заполнять)
    i32 line_height_ = 0; // Высота строки
    std::vector<std::shared_ptr<Texture>> textures_; // Текстурные атласы с глифами
    std::unordered_map<c32, SFGlyph> glyphs_; // Кодовая позиция → область в аталасе

public:
    SpriteFont(const SpriteFont&) = delete;
    SpriteFont& operator =(const SpriteFont&) = delete;

    SpriteFont(const fs::path& path);

    // Генерирует спрайтовый шрифт из векторного
    template <typename T>
    SpriteFont(const T& settings, i64* generation_time_ms = nullptr);

    const std::vector<std::shared_ptr<Texture>>& textures() const { return textures_; }
    const std::unordered_map<c32, SFGlyph>& glyphs() const { return glyphs_; }

    // Если глифа нет, то возвращает глиф для '?', который обязан быть в шрифте
    const SFGlyph& get_glyph(c32 code_point);

    i32 line_height() const { return line_height_; }

    // Расширение должно быть .fnt
    void save(const fs::path& path) const;
};

} // namespace dviglo
