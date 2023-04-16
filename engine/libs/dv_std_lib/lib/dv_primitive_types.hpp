// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include <bit>     // std::endian
#include <cstddef> // std::byte
#include <cstdint> // int8_t, ...


// Пользователь может инжектировать dvt (dviglo types) в любое пространство имён.
// Для namespace используется короткое имя вместо dviglo::primitive_types, так как в
// подсказах VS тип пишется вместе с пространством имён
namespace dvt
{

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

// Unicode code point (UTF-32 code unit)
using c32 = char32_t;

// Для сырых данных
using std::byte;

// Некий хеш (например контрольная сумма)
using hash16 = u16;
using hash32 = u32;
using hash64 = u64;

// Некий идентификатор
using id32 = u32;

// Некая маска
using mask32 = u32;

// Некий набор битовых флагов
using flagset32 = u32;

} // namespace dvt


namespace dviglo
{

using namespace dvt;

} // namespace dviglo


// Dviglo поддерживает только 64-битные платформы
static_assert(sizeof(void*) == 8);

// Dviglo поддерживает только little-endian
static_assert(std::endian::native == std::endian::little);
