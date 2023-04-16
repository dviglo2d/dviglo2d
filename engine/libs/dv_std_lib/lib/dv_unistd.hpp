// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_platform.hpp"
#include "dv_primitive_types.hpp"

#include <fcntl.h> // open(...)

#if DV_WINDOWS_MSVC
    #include <io.h>

    #define DV_O_CREAT  _O_CREAT
    #define DV_O_TRUNC  _O_TRUNC
    #define DV_O_WRONLY _O_WRONLY

    #define DV_S_IRUSR _S_IREAD
    #define DV_S_IWUSR _S_IWRITE

    // Используется только в Linux
    #define DV_S_IRGRP (DV_S_IRUSR >> 3)
    #define DV_S_IWGRP (DV_S_IWUSR >> 3)

    // Используется только в Linux
    #define DV_S_IROTH (DV_S_IRGRP >> 3)
    #define DV_S_IWOTH (DV_S_IWGRP >> 3)

    using dv_mode_t = dvt::i32;

    #define DV_STDIN_FILENO  0
    #define DV_STDOUT_FILENO 1
    #define DV_STDERR_FILENO 2
#elif DV_WINDOWS_MINGW || DV_LINUX_GCC || DV_LINUX_CLANG
    #include <unistd.h>

    #define DV_O_CREAT  O_CREAT
    #define DV_O_TRUNC  O_TRUNC
    #define DV_O_WRONLY O_WRONLY

    #define DV_S_IRUSR S_IRUSR
    #define DV_S_IWUSR S_IWUSR

    #define DV_S_IRGRP S_IRGRP
    #define DV_S_IWGRP S_IWGRP

    #define DV_S_IROTH S_IROTH
    #define DV_S_IWOTH S_IWOTH

    using dv_mode_t = mode_t;

    #define DV_STDIN_FILENO  STDIN_FILENO
    #define DV_STDOUT_FILENO STDOUT_FILENO
    #define DV_STDERR_FILENO STDERR_FILENO
#else
    #error
#endif

#if DV_WINDOWS_MSVC || DV_WINDOWS_MINGW
    #define DV_O_TEXT      _O_TEXT
    #define DV_O_BINARY    _O_BINARY
    #define DV_O_NOINHERIT _O_NOINHERIT
#elif DV_LINUX_GCC || DV_LINUX_CLANG
    // Используется только в Windows
    #define DV_O_TEXT      0
    #define DV_O_BINARY    0
    #define DV_O_NOINHERIT 0 // Аналог O_CLOEXEC
#endif


namespace dviglo
{

// https://man7.org/linux/man-pages/man2/open.2.html
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/open-wopen
i32 fd_open(const char* path, i32 flags);
i32 fd_open(const char* path, i32 flags, dv_mode_t mode);

// https://man7.org/linux/man-pages/man2/close.2.html
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/close
i32 fd_close(i32 fd);

// https://man7.org/linux/man-pages/man2/write.2.html
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/write
i32 fd_write(i32 fd, const void* buffer, u32 num_bytes);

// https://man7.org/linux/man-pages/man2/read.2.html
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/read
i32 fd_read(i32 fd, void* buffer, u32 buffer_size);

// https://man7.org/linux/man-pages/man2/dup.2.html
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/dup-dup2
i32 fd_dup(i32 old_fd);
i32 fd_dup2(i32 old_fd, i32 new_fd);

// Создаёт два связанных файловых дескриптора:
// в 0-й элемент массива записывает дескриптор, открытый на чтение, а в 1-й - открытый на запись.
// Параметры psize и textmode используются только в Windows
// https://man7.org/linux/man-pages/man2/pipe.2.html
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/pipe
i32 fd_pipe(i32 out_fds[2], u32 pipe_size = 4096, i32 text_mode = 0);
inline constexpr i32 pipe_read_side = 0;
inline constexpr i32 pipe_write_side = 1;


// RAII-обёртка для файлового дескриптора
class FileDescriptor
{
private:
    i32 file_descriptor_ = -1;

public:
    FileDescriptor(i32 file_descriptor)
        : file_descriptor_(file_descriptor)
    {
    }

    FileDescriptor(const char* path, i32 flags)
    {
        open(path, flags);
    }

    FileDescriptor(const char* path, i32 flags, dv_mode_t mode)
    {
        open(path, flags, mode);
    }

    // Запрещаем копирование
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    // Разрешаем перемещение
    FileDescriptor(FileDescriptor&& other) noexcept
        : file_descriptor_(other.file_descriptor_)
    {
        other.file_descriptor_ = -1;
    }

    FileDescriptor& operator =(FileDescriptor&& other) noexcept
    {
        if (this != &other)
        {
            close();
            file_descriptor_ = other.file_descriptor_;
            other.file_descriptor_ = -1;
        }

        return *this;
    }

    ~FileDescriptor()
    {
        close();
    }

    i32 file_descriptor() const { return file_descriptor_; }

    bool is_open() const { return file_descriptor_ >= 0; }

    i32 open(const char* path, i32 flags)
    {
        if (is_open())
            close();

        file_descriptor_ = fd_open(path, flags);
        return file_descriptor_;
    }

    i32 open(const char* path, i32 flags, dv_mode_t mode)
    {
        if (is_open())
            close();

        file_descriptor_ = fd_open(path, flags, mode);
        return file_descriptor_;
    }

    i32 close()
    {
        i32 ret = fd_close(file_descriptor_);
        file_descriptor_ = -1;
        return ret;
    }
};

} // namespace dviglo
