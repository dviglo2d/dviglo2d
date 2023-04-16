// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <dv_string.hpp>
#include <SDL3/SDL.h>
#include <utility> // std::exchange()


namespace dviglo
{

// Обёртка над SDL_PropertiesID. Это u32, лучше передавать по значению
class SdlPropsId
{
private:
    SDL_PropertiesID id_ = 0;

public:
    SdlPropsId()
    {
        id_ = SDL_CreateProperties();
    }

    ~SdlPropsId()
    {
        SDL_DestroyProperties(id_); // Проверка на 0 не нужна
        id_ = 0;
    }

    // Запрещаем копирование
    SdlPropsId(const SdlPropsId&) = delete;
    SdlPropsId& operator =(const SdlPropsId&) = delete;

    // Разрешаем перемещение

    SdlPropsId(SdlPropsId&& other) noexcept
        : id_(std::exchange(other.id_, 0))
    {
    }

    SdlPropsId& operator =(SdlPropsId&& other) noexcept
    {
        if (this != &other)
            id_ = std::exchange(other.id_, 0);

        return *this;
    }

    SDL_PropertiesID get() const { return id_; }

    void set_string(const StrUtf8& name, const StrUtf8& value) { SDL_SetStringProperty(id_, name.c_str(), value.c_str()); }
    void set_number(const StrUtf8& name, i64 value) { SDL_SetNumberProperty(id_, name.c_str(), value); }
    void set_boolean(const StrUtf8& name, bool value) { SDL_SetBooleanProperty(id_, name.c_str(), value); }
};

} // namespace dviglo
