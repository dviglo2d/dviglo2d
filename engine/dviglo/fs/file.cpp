// Copyright (c) the Dviglo project
// License: MIT

#include "file.hpp"

#include "file_base.hpp"
#include "log.hpp"

#include <array>

using namespace std;


namespace dviglo
{

// Используем самый быстрый способ: https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
vector<byte> read_all_data(const StrUtf8& path)
{
    vector<byte> ret;

    FILE* fp = file_open(path, "rb");
    if (!fp)
    {
        DV_LOG->writef_error("{} | !fp | {}", DV_FUNCSIG, path);
        return ret;
    }

    // Определяем размер файла и выделяем память
    file_seek(fp, 0, SEEK_END);
    ret.resize(file_tell(fp));
    file_rewind(fp);

    file_read(ret.data(), 1, static_cast<i32>(ret.size()), fp);
    file_close(fp);

    return ret;
}

StrUtf8 read_all_text(const StrUtf8& path)
{
    StrUtf8 ret;

    FILE* fp = file_open(path, "rb");
    if (!fp)
    {
        DV_LOG->writef_error("{} | !fp | {}", DV_FUNCSIG, path);
        return ret;
    }

    // Определяем размер файла и выделяем память
    file_seek(fp, 0, SEEK_END);
    ret.resize(file_tell(fp));
    file_rewind(fp);

    file_read(ret.data(), 1, static_cast<i32>(ret.size()), fp);
    file_close(fp);

    return ret;
}

vector<StrUtf8> read_all_lines(const StrUtf8& path, bool ignore_empty)
{
    vector<StrUtf8> ret;

    FILE* fp = file_open(path, "rb");
    if (!fp)
    {
        DV_LOG->writef_error("{} | !fp | {}", DV_FUNCSIG, path);
        return ret;
    }

    char buffer[1024 * 4];
    StrUtf8 chunk; // Кусок текста, который ещё не разделён на строки

    while (fgets(buffer, sizeof(buffer), fp))
    {
        chunk += buffer;
        size_t start = 0; // Начало очередной строки в chunk

        while (true)
        {
            size_t end = chunk.find('\n', start); // Конец очередной строки в chunk
            if (end == StrUtf8::npos)
                break;

            StrUtf8 line = (end > 1 && chunk[end - 1] == '\r')  // Если оканчивается на \r\n
                         ? chunk.substr(start, end - start - 1) // Не копируем \r\n в результат
                         : chunk.substr(start, end - start);    // Не копируем \n в результат

            if (!(line.empty() && ignore_empty))
                ret.push_back(std::move(line));

            start = end + 1;
        }

        chunk.erase(0, start); // Удаляем из chunk скопированные строки
        start = 0;
    }

    // Последняя строка в файле может быть без \n в конце
    if (chunk.length())
        ret.push_back(chunk);

    return ret;
}

} // namespace dviglo
