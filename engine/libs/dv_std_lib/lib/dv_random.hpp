// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "dv_primitive_types.hpp"

#include <random>


namespace dviglo
{

class Random
{
private:
    std::mt19937 generator_;

public:
    Random(bool random_seed = true)
    {
        if (random_seed)
        {
            // Используем random_device только для генерации seed, так как он медленный
            std::random_device device;
            u32 seed = device();

            // А дальше будем использовать generator_
            generator_.seed(seed);
        }
    }

    // Генерирует случайное число из диапазона [min, max] включительно
    i32 generate(i32 min, i32 max)
    {
        std::uniform_int_distribution<i32> distribution(min, max);
        return distribution(generator_);
    }
};

} // namespace dviglo
