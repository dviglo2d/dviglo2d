// Copyright (c) the Dviglo project
// License: MIT

#include "file.hpp"

#include "file_base.hpp"
#include "log.hpp"

using namespace std;


namespace dviglo
{

// Используем самый быстрый способ: https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
StrUtf8 read_all_text(const StrUtf8& path)
{
    StrUtf8 ret;

    FILE* fp = file_open(path, "rb");

    if (!fp)
    {
        DV_LOG->writef_error("read_all_text(): !fp | path = \"{}\"", path);
        return ret;
    }

    // Определяем размер файла и выделяем память
    file_seek(fp, 0, SEEK_END);
    ret.resize(file_tell(fp));
    file_rewind(fp);

    file_read(ret.data(), 1, (i32)ret.size(), fp);
    file_close(fp);

    return ret;
}

vector<byte> read_all_data(const StrUtf8& path)
{
    vector<byte> ret;

    FILE* fp = file_open(path, "rb");

    if (!fp)
    {
        DV_LOG->writef_error("read_all_data(): !fp | path = \"{}\"", path);
        return ret;
    }

    // Определяем размер файла и выделяем память
    file_seek(fp, 0, SEEK_END);
    ret.resize(file_tell(fp));
    file_rewind(fp);

    file_read(ret.data(), 1, (i32)ret.size(), fp);
    file_close(fp);

    return ret;
}

} // namespace dviglo
