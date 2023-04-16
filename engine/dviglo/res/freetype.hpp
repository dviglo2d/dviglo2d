// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem_base.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H


namespace dviglo
{

// Обёртка над FT_Library
class FreeType final : public SubsystemBase<FreeType>
{
private:
    FT_Library library_ = nullptr; // Это указатель

public:
    FreeType();
    ~FreeType();

    FT_Library library() const { return library_; }
};

} // namespace dviglo

#define DV_FREETYPE (dviglo::FreeType::instance())
