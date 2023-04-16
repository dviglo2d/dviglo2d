// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_log.hpp"


namespace dviglo
{

// Базовый класс для подсистем.
// Используется идиома CRTP
template <class FinalClass>
class SubsystemBase : public SubsystemIndex
{
protected:
    // Инициализируется в конструкторе, если не было критических ошибок, требующих остановки программы
    inline static FinalClass* instance_ = nullptr;

    SubsystemBase()
    {
        // Объект должен быть только один
        assert(!instance_);
    }

    ~SubsystemBase()
    {
        Log::writef_debug("Subsystem {} destructed | {}", index(), DV_FUNC_SIG);
        instance_ = nullptr;
    }

public:
    // Метод используется только для доступа к подсистеме из любого места программы. Это не синглтон Майерса.
    // Объект создаётся и уничтожается снаружи класса, чтобы контролировать порядок создания и уничтожения подсистем
    static FinalClass* instance() { return instance_; }
};

} // namespace dviglo
