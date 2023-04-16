// Copyright (c) the Dviglo project
// License: MIT

#include "shader_program.hpp"

#include "../fs/file.hpp"
#include "../fs/log.hpp"


namespace dviglo
{

// Возвращает ID скомпилированного шейдера или ноль в случае ошибки
static GLuint compile_shader(const StrUtf8& file_path, GLenum type)
{
    StrUtf8 src = read_all_text(file_path); // Читаем файл

    if (src.empty())
        return 0; // Если не удалось прочесть файл, сообщение об ошибке уже выведено в лог

    // Компилируем шейдер
    GLuint gpu_object_name = glCreateShader(type);
    const char* src_c_str = src.c_str();
    glShaderSource(gpu_object_name, 1, &src_c_str, nullptr);
    glCompileShader(gpu_object_name);

    // Успешно ли прошла компиляция
    GLint success;
    glGetShaderiv(gpu_object_name, GL_COMPILE_STATUS, &success);

    // Компилятор может выдавать предупреждения, поэтому проверяем лог даже при успешной компиляции
    GLint log_buffer_size; // Длина строки + нуль-терминатор
    glGetShaderiv(gpu_object_name, GL_INFO_LOG_LENGTH, &log_buffer_size);

    if (log_buffer_size)
    {
        StrUtf8 msg(log_buffer_size - 1, '\0');
        glGetShaderInfoLog(gpu_object_name, log_buffer_size, nullptr, msg.data());
        trim_end_chars(msg, "\n"); // Удаляем перевод строки в конце

        if (success)
            DV_LOG->write_warning(file_path + " | " + msg);
        else
            DV_LOG->write_error(file_path + " | " + msg);
    }

    if (success)
    {
        return gpu_object_name;
    }
    else
    {
        glDeleteShader(gpu_object_name);
        return 0;
    }
}

ShaderProgram::ShaderProgram(const StrUtf8& vertex_shader_path, const StrUtf8& fragment_shader_path,
                             const StrUtf8& geometry_shader_path)
{
    GLuint vertex_shader = compile_shader(vertex_shader_path, GL_VERTEX_SHADER);

    if (!vertex_shader)
        return;

    GLuint fragment_shader = compile_shader(fragment_shader_path, GL_FRAGMENT_SHADER);

    if (!fragment_shader)
    {
        glDeleteShader(vertex_shader);
        return;
    }

    GLuint geometry_shader = 0;

    if (!geometry_shader_path.empty()) // Геометрический шейдер может отсутствовать
    {
        geometry_shader = compile_shader(geometry_shader_path, GL_GEOMETRY_SHADER);

        if (!geometry_shader)
        {
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            return;
        }
    }

    // Линкуем все шейдеры в шейдерную программу
    gpu_object_name_ = glCreateProgram();
    glAttachShader(gpu_object_name_, vertex_shader);
    glAttachShader(gpu_object_name_, fragment_shader);

    if (geometry_shader)
        glAttachShader(gpu_object_name_, geometry_shader);

    glLinkProgram(gpu_object_name_);

    // Успешно ли прошла линковка
    GLint success;
    glGetProgramiv(gpu_object_name_, GL_LINK_STATUS, &success);

    // Компоновщик может выдавать предупреждения, поэтому проверяем лог даже при успешной линковке
    GLint log_buffer_size; // Длина строки + нуль-терминатор
    glGetProgramiv(gpu_object_name_, GL_INFO_LOG_LENGTH, &log_buffer_size);

    if (log_buffer_size)
    {
        StrUtf8 msg(log_buffer_size - 1, '\0');
        glGetProgramInfoLog(gpu_object_name_, log_buffer_size, nullptr, msg.data());
        trim_end_chars(msg, "\n"); // Удаляем перевод строки в конце

        StrUtf8 out_message = vertex_shader_path + " + " + fragment_shader_path;

        if (geometry_shader)
            out_message += " + " + geometry_shader_path;

        out_message += " | " + msg;

        if (success)
            DV_LOG->write_warning(out_message);
        else
            DV_LOG->write_error(out_message);
    }

    // Шейдеры после линковки не нужны
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteShader(geometry_shader); // Проверка на 0 не нужна

    if (!success)
    {
        glDeleteProgram(gpu_object_name_);
        gpu_object_name_ = 0;
    }
}

} // namespace dviglo
