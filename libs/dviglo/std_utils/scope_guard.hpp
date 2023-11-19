// Copyright (c) the Dviglo project
// License: MIT

/*

Вспомогательный объект, который выполнит какую-то функцию, когда выйдет из области видимости.

Пример использования:
{
    ScopeGuard sg1([] { cout << "sg1 "; });
    ScopeGuard sg2 = [] { cout << "sg2 "; };
}

Будет выведено "sg2 sg1 " так как в C++ деструкторы выполняются в реверсном порядке вызовов конструкторов.

*/

#pragma once


namespace dviglo
{

template<typename F>
class ScopeGuard
{
private:
    F func_;

public:
    // Разрешаем только конструктор перемещения
    ScopeGuard(F&& func)
        : func_(func)
    {
    }

    // Всё остальное запрещаем
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;

    // В деструкторе вызываем функцию
    ~ScopeGuard()
    {
        func_();
    }
};

} // namespace dviglo
