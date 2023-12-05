// Copyright (c) the Dviglo project

#pragma once

// GLM опеределяет типы i32, u64 и т.п., которые конфликтуют с типами движка

#define i8  GLM_i8
#define u8  GLM_u8
#define i16 GLM_i16
#define u16 GLM_u16
#define i32 GLM_i32
#define u32 GLM_u32
#define i64 GLM_i64
#define u64 GLM_u64

#include "glm.hpp"

#undef i8
#undef u8
#undef i16
#undef u16
#undef i32
#undef u32
#undef i64
#undef u64
