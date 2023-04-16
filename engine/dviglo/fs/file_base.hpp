// Copyright (c) the Dviglo project
// License: MIT

// Функции для работы с большими файлами (больше 2 GiB).
// Можно использовать до инициализации любых подсистем (не пишут в лог)

#pragma once

#include "path.hpp"

#include <cstdio>


namespace dviglo
{

// https://en.cppreference.com/w/c/io/fopen
inline FILE* file_open(const StrUtf8& filename, const StrUtf8& mode)
{
#if defined(DV_WINDOWS_MSVC) || defined(DV_WINDOWS_MINGW)
    return _wfopen(to_win_native(filename).c_str(), to_wstring(mode).c_str());
#elif defined(DV_LINUX_GCC) || defined(DV_LINUX_CLANG)
    return fopen64(filename.c_str(), mode.c_str());
#else
    #error
#endif
}

// https://en.cppreference.com/w/c/io/fclose
inline i32 file_close(FILE* stream)
{
    return fclose(stream);
}

// https://en.cppreference.com/w/c/io/fflush
inline i32 file_flush(FILE* stream)
{
    return fflush(stream);
}

// https://en.cppreference.com/w/c/io/fread
inline i32 file_read(void* buffer, i32 size, i32 count, FILE* stream)
{
    return static_cast<i32>(fread(buffer, size, count, stream));
}

// https://en.cppreference.com/w/c/io/fwrite
inline i32 file_write(const void* buffer, i32 size, i32 count, FILE* stream)
{
    return static_cast<i32>(fwrite(buffer, size, count, stream));
}

// https://en.cppreference.com/w/c/io/ftell
inline i64 file_tell(FILE* stream)
{
#if defined(DV_WINDOWS_MSVC)
    return _ftelli64(stream);
#elif defined(DV_WINDOWS_MINGW) || defined(DV_LINUX_GCC) || defined(DV_LINUX_CLANG)
    return ftello64(stream);
#else
    #error
#endif
}

// https://en.cppreference.com/w/c/io/fseek
inline i32 file_seek(FILE* stream, i64 offset, i32 origin)
{
#if defined(DV_WINDOWS_MSVC)
    return _fseeki64(stream, offset, origin);
#elif defined(DV_WINDOWS_MINGW) || defined(DV_LINUX_GCC) || defined(DV_LINUX_CLANG)
    return fseeko64(stream, offset, origin);
#else
    #error
#endif
}

// https://en.cppreference.com/w/cpp/io/c/rewind
inline void file_rewind(FILE* stream)
{
    return rewind(stream);
}

} // namespace dviglo
