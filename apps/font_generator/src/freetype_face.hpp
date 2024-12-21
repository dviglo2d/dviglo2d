#pragma once

#include "font_settings.hpp"

#include <dviglo/fs/file.hpp>
#include <dviglo/fs/log.hpp>
#include <dviglo/res/freetype.hpp>

#include <format>

using namespace dviglo;
using namespace std;


// Шрифт
class FreeTypeFace
{
private:
    FT_Face face_ = nullptr; // Это указатель

    // Данные нужно держать в памяти до вызова FT_Done_Face()
    vector<byte> data;

public:
    FreeTypeFace(const FontSettings& font_settings)
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
            DV_LOG->write_error(format("FreeTypeFace::FreeTypeFace(): FT_New_Memory_Face() error {}", error));
            return;
        }

        error = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
        if (error)
        {
            DV_LOG->write_error(format("FreeTypeFace::FreeTypeFace(): FT_Select_Charmap() error {}", error));
            return;
        }

        // Шрифт может содержать несколько начертаний, но используется только первое
        if (face_->num_faces != 1)
            DV_LOG->write_warning(format("FreeTypeFace::FreeTypeFace(): face_->num_faces != 1 | {}", face_->num_faces));

        error = FT_Set_Pixel_Sizes(face_, 0, font_settings.height);
        if (error)
        {
            DV_LOG->write_error(format("FreeTypeFace::FreeTypeFace(): FT_Set_Pixel_Sizes() error {}", error));
            return;
        }
    }

    ~FreeTypeFace()
    {
        if (face_)
        {
            FT_Error error = FT_Done_Face(face_);
            if (error)
                DV_LOG->write_error(format("FreeTypeFace::~FreeTypeFace(): error {}", error));
        }
    }

    // Запрещаем копирование
    FreeTypeFace(const FreeTypeFace&) = delete;
    FreeTypeFace& operator=(const FreeTypeFace&) = delete;

    FT_Face get() const { return face_; }
};
