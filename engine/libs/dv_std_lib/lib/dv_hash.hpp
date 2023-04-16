// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <bit>         // std::rotl
#include <functional>  // std::hash
#include <type_traits> // std::is_same_v


namespace dviglo
{

// Цель функции - чтобы при изменении одного бита входного числа каждый бит результата менялся с вероятностью 50%
// (strict avalanche criterion).
// Это делает последовательности 1, 2, 3, ... более равномерно распределёнными.
// Другие способы: https://github.com/boostorg/container_hash/blob/master/include/boost/container_hash/detail/hash_mix.hpp
constexpr size_t hash_mix(size_t x)
{
    // Эти константы подходят только для 64-битных типов
    static_assert(sizeof(size_t) == 8);

    x ^= x >> 27;
    x *= 0x3c79ac492ba7b653uz;
    x ^= x >> 33;
    x *= 0x1c69b3f74ac4ae35uz;
    x ^= x >> 27;

    return x;
}

// Объединяет два хэша в один. Хэши уже должны быть качественными (с перемешанными битами)
constexpr void hash_simple_combine(size_t& result, size_t mixed_hash)
{
    result = std::rotl(result, 17) ^ mixed_hash;
}

// Вычисляет хэш value и комбинирует с результатом
template <typename T>
constexpr void hash_combine(size_t& result, const T& value)
{
    size_t hash = std::hash<T>{}(value);
    hash_simple_combine(result, hash_mix(hash));
}

} // namespace dviglo
