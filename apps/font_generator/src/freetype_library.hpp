// Обёртка над FT_Library

#pragma once

#include <dviglo/fs/log.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <format>

using namespace dviglo;
using namespace std;


// Библиотека FreeType
class FreeTypeLibrary
{
private:
    FT_Library library_ = nullptr; // Это указатель

public:
    FreeTypeLibrary()
    {
        FT_Error error = FT_Init_FreeType(&library_);

        if (error)
            DV_LOG->write_error(format("FreeTypeLibrary::FreeTypeLibrary(): error | {}", error));
        else
            DV_LOG->write_debug("FreeTypeLibrary constructed");
    }

    ~FreeTypeLibrary()
    {
        if (library_)
        {
            FT_Error error = FT_Done_FreeType(library_);

            if (error)
                DV_LOG->write_error(format("FreeTypeLibrary::~FreeTypeLibrary(): error | {}", error));
            else
                DV_LOG->write_debug("FreeTypeLibrary destructed");
        }
    }

    // Запрещаем копирование
    FreeTypeLibrary(const FreeTypeLibrary&) = delete;
    FreeTypeLibrary& operator=(const FreeTypeLibrary&) = delete;

    FT_Library get() const { return library_; }
};
