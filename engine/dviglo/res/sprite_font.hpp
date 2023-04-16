// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "../gl_utils/texture.hpp"
#include "../math/rect.hpp"
#include "../std_utils/string.hpp"

#include <limits>
#include <unordered_map>


namespace dviglo
{

struct Glyph
{
    // Область текстуры с глифом
    IntRect rect;

    // Смещение от origin (левый верхний угол) при рендеринге.
    // offset.x = horiBearingX
    // offset.y = ascender - horiBearingY
    // https://freetype.org/freetype2/docs/glyphs/glyphs-3.html
    glm::ivec2 offset{0, 0};

    // Расстояние между origin (левый верхний угол) текущего глифа и origin следующего глифа при рендеринге
    i32 advance_x = 0;

    // Номер текстуры
    i32 page = std::numeric_limits<i32>::max();
};


struct SFSettings
{
    StrUtf8 src_path;
    i32 height; // В пикселях
    bool anti_aliasing;
    glm::ivec2 texture_size;

    SFSettings(const StrUtf8& src_path,
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

struct SFSettingsSimple : SFSettings
{
    i32 blur_radius;
    u32 color; // 0xAABBGGRR

    SFSettingsSimple(const StrUtf8& src_path,
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

struct SFSettingsContour : SFSettings
{
    f32 thickness;
    i32 blur_radius;
    u32 color; // 0xAABBGGRR

    SFSettingsContour(const StrUtf8& src_path,
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

struct SFSettingsOutlined : SFSettings
{
    u32 main_color; // 0xAABBGGRR
    u32 outline_color; // 0xAABBGGRR
    f32 outline_thickness;
    i32 outline_blur_radius;

    SFSettingsOutlined(const StrUtf8& src_path,
                       i32 height = 20,
                       u32 main_color = 0xFFFFFFFF,
                       u32 outline_color = 0xFF000000,
                       f32 outline_thickness = 2,
                       i32 outline_blur_radius = 0,
                       bool anti_aliasing = true,
                       glm::ivec2 texture_size = glm::ivec2(1024, 1024))
        : SFSettings(src_path, height, anti_aliasing, texture_size)
        , main_color(main_color)
        , outline_color(outline_color)
        , outline_thickness(outline_thickness)
        , outline_blur_radius(outline_blur_radius)
    {
    }
};

class SpriteFont
{
private:
    StrUtf8 face_; // Название исходного шрифта (из которого был сгенерирован растровый шрифт)
    i32 size_ = 0; // Размер исходного шрифта
    i32 line_height_ = 0; // Высота растрового шрифта
    std::vector<std::shared_ptr<Texture>> textures_; // Текстурные атласы с символами
    std::unordered_map<c32, Glyph> glyphs_; // Кодовая позиция : изображение

public:
    SpriteFont(const SpriteFont&) = delete;
    SpriteFont& operator=(const SpriteFont&) = delete;

    SpriteFont(const StrUtf8& file_path);

    // Генерирует спрайтовый шрифт из ttf
    SpriteFont(const SFSettingsSimple& settings, i64* generation_time_ms = nullptr);
    SpriteFont(const SFSettingsContour& settings, i64* generation_time_ms = nullptr);
    SpriteFont(const SFSettingsOutlined& settings, i64* generation_time_ms = nullptr);

    const std::vector<std::shared_ptr<Texture>>& textures() const { return textures_; }
    const std::unordered_map<c32, Glyph>& glyphs() const { return glyphs_; }
    i32 line_height() const { return line_height_; }

    void save(const StrUtf8& file_path) const;
};

} // namespace dviglo
