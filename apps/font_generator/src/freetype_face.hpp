#pragma once

#include "freetype_library.hpp"

#include <dviglo/fs/file.hpp>


// Шрифт
class FreeTypeFace
{
private:
    FT_Face face_ = nullptr; // Это указатель

public:
    FreeTypeFace(FreeTypeLibrary& lib, const StrUtf8& font_path, u32 font_height)
    {
        // Не используем FT_New_Face(), так как она ожидает путь в кодировке ANSI
        vector<byte> data = read_all_data(font_path);

        if (data.empty())
        {
            DV_LOG->write_error("FreeTypeFace::FreeTypeFace(): data.empty()");
            return;
        }

        FT_Error error = FT_New_Memory_Face(lib.get(), (const FT_Byte*)data.data(), (FT_Long)data.size(), 0, &face_);

        if (error)
        {
            DV_LOG->write_error(format("FreeTypeFace::FreeTypeFace(): FT_New_Memory_Face() error | {}", error));
            return;
        }

        // Шрифт может содержать несколько начертаний, но используется только первое
        if (face_->num_faces != 1)
            DV_LOG->write_warning(format("FreeTypeFace::FreeTypeFace(): face_->num_faces != 1 | {}", face_->num_faces));

        error = FT_Set_Pixel_Sizes(face_, 0, font_height);

        if (error)
        {
            DV_LOG->write_error(format("FreeTypeFace::FreeTypeFace(): FT_Set_Pixel_Sizes() error | {}", error));
            return;
        }

        DV_LOG->write_debug("FreeTypeFace constructed");
    }

    ~FreeTypeFace()
    {
        if (face_)
        {
            FT_Error error = FT_Done_Face(face_);

            if (error)
                DV_LOG->write_error(format("FreeTypeFace::~FreeTypeFace(): error | {}", error));
            else
                DV_LOG->write_debug("FreeTypeFace destructed");
        }
    }

    // Запрещаем копирование
    FreeTypeFace(const FreeTypeFace&) = delete;
    FreeTypeFace& operator=(const FreeTypeFace&) = delete;

    FT_Face get() { return face_; }
};
