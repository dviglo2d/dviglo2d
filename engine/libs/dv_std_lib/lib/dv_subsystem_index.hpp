// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_primitive_types.hpp"

#include <cassert>
#include <concepts> // std::movable


namespace dviglo
{

// Базовый класс для подсистем.
// Проверяет, что подсистемы уничтожаются в порядке, обратном созданию
class SubsystemIndex
{
private:
    // Число созданных подсистем
    inline static i32 count_ = 0;

    // Порядковый номер подсистемы
    i32 index_;

protected:
    SubsystemIndex()
    {
        index_ = count_;
        ++count_;
    }

    // Деструктор намеренно не virtual
    virtual ~SubsystemIndex()
    {
        assert(count_ > 0);
        --count_;
        assert(index_ == count_);
    }

    // Запрещаем копирование
    SubsystemIndex(const SubsystemIndex&) = delete;
    SubsystemIndex& operator =(const SubsystemIndex&) = delete;

    // Перемещать тоже нельзя
    static_assert(!std::movable<SubsystemIndex>);

public:
    // Порядковый номер подсистемы
    i32 index() const { return index_; }
};

} // namespace dviglo
