// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_subsystem.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H


namespace dviglo
{

// Обёртка над FT_Library
class FreeType final : public Subsystem
{
private:
    // Инициализируется в конструкторе
    inline static FreeType* instance_ = nullptr;

    FT_Library library_ = nullptr; // Это указатель

public:
    static FreeType* instance()
    {
        assert(instance_);
        return instance_;
    }

    FreeType();
    ~FreeType();

    // Запрещаем копирование
    FreeType(const FreeType&) = delete;
    FreeType& operator =(const FreeType&) = delete;

    FT_Library library() const { return library_; }
};

} // namespace dviglo

#define DV_FREETYPE (dviglo::FreeType::instance())
