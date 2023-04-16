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

Чтобы не придумывать имена переменных, можно использовать макрос: DV_SCOPE_GUARD = [] { cout << "sg3 "; };
*/

#pragma once


namespace dviglo
{

template <typename F>
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
    ScopeGuard& operator =(const ScopeGuard&) = delete;
    ScopeGuard& operator =(ScopeGuard&&) = delete;

    // В деструкторе вызываем функцию
    ~ScopeGuard()
    {
        func_();
    }
};

#define DV_CONCATENATE_IMPL(s1, s2) s1##s2
#define DV_CONCATENATE(s1, s2) DV_CONCATENATE_IMPL(s1, s2)
#define DV_SCOPE_GUARD ScopeGuard DV_CONCATENATE(scope_guard_, __LINE__)

} // namespace dviglo
