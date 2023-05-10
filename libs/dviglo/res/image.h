// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../std_utils/str.h"


namespace dviglo
{

class DV_API Image
{
private:
    i32 width_;
    i32 height_;
    i32 num_components_;
    byte* data_;

public:
    Image(const StrUtf8& path);
    ~Image();

    i32 width() const { return width_; }
    i32 height() const { return height_; }
    i32 num_components() const { return num_components_; }
    const byte* data() const { return data_; }
};

} // namespace dviglo
