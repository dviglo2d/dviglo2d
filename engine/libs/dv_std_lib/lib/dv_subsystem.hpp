// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_primitive_types.hpp"

#include <cassert>


#ifndef NDEBUG

namespace dviglo
{

// Базовый класс для подсистем.
// Проверяет, что подсистемы уничтожаются в порядке, обратном созданию
class Subsystem
{
private:
    // Число созданных подсистем
    inline static i32 count_ = 0;

    // Порядковый номер текущей подсистемы
    i32 index_;

protected:
    Subsystem()
    {
        index_ = count_;
        ++count_;
    }

    // Деструктор намеренно не virtual
    ~Subsystem()
    {
        assert(count_ > 0);
        --count_;
        assert(index_ == count_);
    }
};

} // namespace dviglo

#endif // ndef NDEBUG
