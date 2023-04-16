// Copyright (c) the Dviglo project
// License: MIT

#pragma once

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

    void set_string(const char* name, const char* value) { SDL_SetStringProperty(id_, name, value); }
    void set_number(const char* name, Sint64 value) { SDL_SetNumberProperty(id_, name, value); }
    void set_boolean(const char* name, bool value) { SDL_SetBooleanProperty(id_, name, value); }
};

} // namespace dviglo
