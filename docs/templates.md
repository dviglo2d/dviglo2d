# Шаблоны классов

## Подсистема

my_subsystem.hpp

```
#pragma once

#include <dv_subsystem.hpp>

using namespace dviglo;


class MySubsystem final : public Subsystem
{
private:
    // Инициализируется в конструкторе
    inline static MySubsystem* instance_ = nullptr;

public:
    static MySubsystem* instance()
    {
        assert(instance_);
        return instance_;
    }

    MySubsystem();
    ~MySubsystem();

    // Запрещаем копирование
    MySubsystem(const MySubsystem&) = delete;
    MySubsystem& operator =(const MySubsystem&) = delete;
};

#define MY_SUBSYSTEM (MySubsystem::instance())
```

my_subsystem.cpp

```
#include "my_subsystem.hpp"

#include <dv_log.hpp>


MySubsystem::MySubsystem()
{
    assert(!instance_);

    // Ваш код

    instance_ = this;
    Log::write_debug("MySubsystem constructed");
}

MySubsystem::~MySubsystem()
{
    instance_ = nullptr;

    // Ваш код

    Log::write_debug("MySubsystem destructed");
}
```

Метод instance() используется только для доступа к подсистеме из любого места программы. Это не синглтон Майерса.
Объект создаётся и уничтожается снаружи класса. Это сделано для того, чтобы контролировать порядок создания и уничтожения подсистем.
