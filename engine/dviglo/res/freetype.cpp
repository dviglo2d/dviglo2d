// Copyright (c) the Dviglo project
// License: MIT

#include "freetype.hpp"

#include "../fs/log.hpp"

#include <cassert>


namespace dviglo
{

FreeType::FreeType()
{
    assert(!instance_);

    FT_Error error = FT_Init_FreeType(&library_);
    if (error)
        DV_LOG->writef_error("FreeType::FreeType(): FT_Init_FreeType() error {}", error);

    instance_ = this;
    DV_LOG->write_debug("FreeType constructed");
}

FreeType::~FreeType()
{
    instance_ = nullptr;

    if (library_)
    {
        FT_Error error = FT_Done_FreeType(library_);
        if (error)
            DV_LOG->writef_error("FreeType::~FreeType(): FT_Done_FreeType() error {}", error);
    }

    DV_LOG->write_debug("FreeType destructed");
}

} // namespace dviglo
