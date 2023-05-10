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

    // Запрещаем копировать объект, так как если в одной из копий будет вызван деструктор,
    // все другие объекты будут иметь ссылку на уничтоженный data_
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Но разрешаем перемещение, чтобы было можно хранить объект в векторе
    Image(Image&&) = default;
    Image& operator=(Image&&) = default;

    i32 width() const { return width_; }
    i32 height() const { return height_; }
    i32 num_components() const { return num_components_; }
    const byte* data() const { return data_; }
};

} // namespace dviglo
