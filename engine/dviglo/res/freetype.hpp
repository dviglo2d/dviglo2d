// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H


namespace dviglo
{

// Обёртка над FT_Library
class FreeType
{
private:
    // Инициализируется в конструкторе
    inline static FreeType* instance_ = nullptr;

    FT_Library library_ = nullptr; // Это указатель

public:
    static FreeType* instance() { return instance_; }

    FreeType();
    ~FreeType();

    FT_Library library() const { return library_; }
};

#define DV_FREETYPE (dviglo::FreeType::instance())

} // namespace dviglo
