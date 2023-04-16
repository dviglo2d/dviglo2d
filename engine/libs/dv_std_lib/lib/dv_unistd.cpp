// Copyright (c) the Dviglo project
// License: MIT

#include "dv_unistd.hpp"

#if DV_WINDOWS_MSVC
    #include <cassert>
    #include <crtdbg.h> // _CrtSetReportMode
#endif


namespace dviglo
{

#if DV_WINDOWS_MSVC

// В Linux и MinGW функции просто возвращают ошибку, а в Windows CRT останавливают программу.
// Избегаем остановки программы
class ReportSilencer
{
private:
    static void __cdecl empty_handler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t)
    {
    }

    _invalid_parameter_handler old_handler_;

public:
    ReportSilencer()
    {
        old_handler_ = _set_thread_local_invalid_parameter_handler(empty_handler);

        // Блокирующее окно должно быть отключено пользователем.
        // Не пытаемся сами временно менять режим, так как это глобальное состояние
        // и будут проблемы при многопоточности
        assert(!(_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_REPORT_MODE) & _CRTDBG_MODE_WNDW));
    }

    ~ReportSilencer()
    {
         _set_thread_local_invalid_parameter_handler(old_handler_);
    }
};

#endif // DV_WINDOWS_MSVC


i32 fd_open(const char* path, i32 flags)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _open(path, flags);
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    return open(path, flags);
#endif
}

i32 fd_open(const char* path, i32 flags, dv_mode_t mode)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _open(path, flags, mode);
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    return open(path, flags, mode);
#endif
}

i32 fd_close(i32 fd)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _close(fd);
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    return close(fd);
#endif
}

i32 fd_write(i32 fd, const void* buffer, u32 num_bytes)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _write(fd, buffer, num_bytes);
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    return write(fd, buffer, num_bytes);
#endif
}

i32 fd_read(i32 fd, void* buffer, u32 buffer_size)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _read(fd, buffer, buffer_size);
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    return read(fd, buffer, buffer_size);
#endif
}

i32 fd_dup(i32 old_fd)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _dup(old_fd);
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    return dup(old_fd);
#endif
}

i32 fd_dup2(i32 old_fd, i32 new_fd)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _dup2(old_fd, new_fd);
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    return dup2(old_fd, new_fd);
#endif
}

i32 fd_pipe(i32 out_fds[2], u32 pipe_size, i32 text_mode)
{
#if DV_WINDOWS_MSVC
    ReportSilencer report_silencer;
    return _pipe(out_fds, pipe_size, text_mode);
#elif DV_WINDOWS_MINGW
    return _pipe(out_fds, pipe_size, text_mode);
#elif DV_LINUX_GCC || DV_LINUX_CLANG
    (void)pipe_size;
    (void)text_mode;
    return pipe(out_fds);
#endif
}

} // namespace dviglo
