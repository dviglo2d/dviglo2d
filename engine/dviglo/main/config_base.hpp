// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_fs.hpp>
#include <dv_subsystem_index.hpp>
#include <glm/glm.hpp>


namespace dviglo
{

enum class WindowMode : u32
{
    windowed = 0,
    resizable,
    maximized,
    fullscreen, // Безрамочное окно
    exclusive_fullscreen // TODO: Возможно удалить
};


class ConfigBase : public SubsystemIndex
{
protected:
    StrUtf8 window_title_{"Игра"};
    glm::ivec2 window_size_{800, 600};
    WindowMode window_mode_ = WindowMode::windowed;

    // 0 - выключено, 1 - включено,
    // -1 - адаптивная вертикальная синхронизация (если не поддерживается, то включается обычная).
    // Значения 2, -2, 3, -3 и т.д. делят частоту кадров
    i32 vsync_ = 0;

    // 0 или 1 - MSAA выключено,
    // другое значение - число сэмплов (рекомендуется 4 или 8).
    // Подробнее: https://habr.com/ru/articles/351706/
    i32 msaa_samples_ = 0;

public:
    // Пользователь может переопределить этот метод.
    // Метод статический, так как текущая подсистема содаётся после открытия лога
    static fs::path log_path() { return get_pref_path("dviglo2d", "default") / "log.log"; }

    StrUtf8 window_title() const { return window_title_; }
    glm::ivec2 window_size() const { return window_size_; }
    WindowMode window_mode() const { return window_mode_; }
    i32 vsync() const { return vsync_; }
    i32 msaa_samples() const { return msaa_samples_; }
};

} // namespace dviglo
