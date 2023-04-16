// Copyright (c) the Dviglo project
// License: MIT

// Функции, которые можно использовать до инициализации любых подсистем

#include "fs_base.hpp"

#include "path.hpp"

#include <memory>

#ifdef _WIN32
    #include <shlobj.h> // SHGetKnownFolderPath()
#else // Linux
    #include <sys/stat.h> // mkdir(), stat()
#endif

using namespace std;


namespace dviglo
{

bool dir_exists(const StrUtf8& path)
{
#ifndef _WIN32
    // Возвращаем true для корневой папки
    if (path == "/")
        return true;
#endif

#ifdef _WIN32
    DWORD attributes = GetFileAttributesW(to_win_native(path).c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return false;
#else
    struct stat st{};

    if (stat(path.c_str(), &st) || !(st.st_mode & S_IFDIR))
        return false;
#endif

    return true;
}

bool create_dir_silent(const StrUtf8& path)
{
    // Рекурсивно создаём родительские папки
    StrUtf8 parent_path = get_parent(path);

    if (parent_path.length() > 1 && !dir_exists(parent_path))
    {
        if (!create_dir_silent(parent_path))
            return false;
    }

#ifdef _WIN32
    bool success = CreateDirectoryW(to_win_native(path).c_str(), NULL)
                   || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    // S_IRWXU == 0700
    bool success = mkdir(path.c_str(), S_IRWXU) == 0
                   || errno == EEXIST;
#endif

    return success;
}

StrUtf8 get_pref_path(StrViewUtf8 org, StrViewUtf8 app)
{
    if (app.empty())
        return StrUtf8();

#ifdef _WIN32
    // %APPDATA% == %USERPROFILE%\AppData\Roaming
    wchar_t* wpath = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &wpath);
    StrUtf8 ret = from_wstring(wpath);
    CoTaskMemFree(wpath);
    ret = to_internal(ret) + "/";
#else
    // https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    char* home = getenv("XDG_DATA_HOME");

    if (!home)
        home = getenv("HOME");

    StrUtf8 ret(home);
    ret += "/.local/share/";
#endif

    if (!org.empty())
    {
        ret += org;
        ret += '/';
    }

    ret += app;
    ret += '/';

    if (!create_dir_silent(ret))
        return StrUtf8();

    return ret;
}

StrUtf8 get_base_path()
{
#ifdef _WIN32
    DWORD buffer_size = 128;

    while (true)
    {
        unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(buffer_size);
        DWORD len = GetModuleFileNameW(nullptr, buffer.get(), buffer_size);

        if (!len) // Возникла ошибка
            return StrUtf8();

        if (len == buffer_size)
        {
            // Буфер слишком маленький. Удваиваем его размер и пробуем ещё раз
            buffer_size *= 2;
            continue;
        }

        // Ищем последний `\\`, отбрасывая то, что после него
        while (len > 0)
        {
            if (buffer[len - 1] == '\\')
                break;

            --len;
        }

        return to_internal(from_wstring(wstring_view(buffer.get(), len)));
    }
#else // Linux
    size_t buffer_size = 128;

    while (true)
    {
        unique_ptr<char[]> buffer = make_unique<char[]>(buffer_size);
        ssize_t len = readlink("/proc/self/exe", buffer.get(), buffer_size);

        if (len <= 0) // Возникла ошибка или функция вернула пустую строку
            return StrUtf8();

        if ((size_t)len == buffer_size)
        {
            // Буфер слишком маленький. Удваиваем его размер и пробуем ещё раз
            buffer_size *= 2;
            continue;
        }

        // Ищем последний `/`, отбрасывая то, что после него
        while (len > 0)
        {
            if (buffer[len - 1] == '/')
                break;

            --len;
        }

        return StrUtf8(buffer.get(), len);
    }
#endif // def _WIN32
}

} // namespace dviglo
