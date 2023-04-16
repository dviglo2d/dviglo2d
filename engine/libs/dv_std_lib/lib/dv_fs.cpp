// Copyright (c) the Dviglo project
// License: MIT

#include "dv_fs.hpp"

#include <memory>

#if defined(DV_WINDOWS)
    #include <shlobj.h> // SHGetKnownFolderPath()
#endif

using namespace std;


namespace dviglo
{

fs::path get_pref_path(StrViewUtf8 org, StrViewUtf8 app)
{
    if (app.empty())
        return fs::path();

#if defined(DV_WINDOWS)
    // %APPDATA% == %USERPROFILE%\AppData\Roaming
    wchar_t* wpath = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &wpath);
    fs::path ret(wpath);
    CoTaskMemFree(wpath);
#elif defined(DV_LINUX)
    // https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    char* home = getenv("XDG_DATA_HOME");

    if (!home)
        home = getenv("HOME");

    fs::path ret(home);
    ret /= ".local/share";
#else
    #error
#endif

    if (!org.empty())
        ret /= org;

    ret /= app;

    if (dir_exists(ret))
        return ret;

    if (create_dirs(ret))
        return ret;
    else
        return fs::path();
}

fs::path get_exe_path()
{
#if defined(DV_WINDOWS)
    DWORD buffer_size = 128;

    while (true)
    {
        unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(buffer_size);
        DWORD len = GetModuleFileNameW(nullptr, buffer.get(), buffer_size);

        if (!len) // Возникла ошибка
            return fs::path();

        if (len == buffer_size)
        {
            // Буфер слишком маленький. Удваиваем его размер и пробуем ещё раз
            buffer_size *= 2;
            continue;
        }

        return fs::path(wstring_view(buffer.get(), len));
    }
#elif defined(DV_LINUX)
    size_t buffer_size = 128;

    while (true)
    {
        unique_ptr<char[]> buffer = make_unique<char[]>(buffer_size);
        ssize_t len = readlink("/proc/self/exe", buffer.get(), buffer_size);

        if (len <= 0) // Возникла ошибка или функция вернула пустую строку
            return fs::path();

        if ((size_t)len == buffer_size)
        {
            // Буфер слишком маленький. Удваиваем его размер и пробуем ещё раз
            buffer_size *= 2;
            continue;
        }

        return fs::path(string_view(buffer.get(), len));
    }
#else
    #error
#endif
}

} // namespace dviglo
