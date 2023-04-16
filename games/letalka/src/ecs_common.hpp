#pragma once

#include <dv_primitive_types.hpp>
#include <dviglo/math/rect.hpp>
#include <glm/glm.hpp>

using namespace dviglo;
using namespace glm;


// ===================== Утилиты =====================

struct Collider
{
    // Позиция центра коллайдера в локальных координатах
    // (относительно центра спрайта)
    vec2 pos;

    // Половинный размер коллайдера
    vec2 half_size;

    Collider() = default;

    Collider(vec2 pos, vec2 half_size)
        : pos(pos)
        , half_size(half_size)
    {
    }

    Collider(f32 x, f32 y, f32 half_width, f32 half_height)
        : pos(x, y)
        , half_size(half_width, half_height)
    {
    }

    static const Collider zero;
};

inline const Collider Collider::zero(0.f, 0.f, 0.f, 0.f);

// ===================== Компоненты =====================

// Origin объекта - центр спрайта
struct CObject
{
    // Экранные координаты origin-а объекта
    vec2 pos{0.f, 0.f};

    // Область спрайта в текстуре
    Rect uv = Rect::zero;

    // Прямоугольное физическое тело
    Collider collider = Collider::zero;
};

// Скорость есть у врагов и проджектайлов, а у игрока её нет
struct CVelocity
{
    // Пикселей в секунду
    vec2 value{0.f, 0.f};
};

// Уничтоженный объект не апдейтится и не рендерится.
// Он будет удалён из реестра в конце апдейта
struct CDestroyedMarker
{
};

// ===================== Системы =====================

// Перемещает объекты и уничтожает их, если они далеко за границей экрана
void s_apply_velocities(i64 ns);

void s_draw_colliders();

// ===================== Утилиты =====================

// Определяет наложение коллайдеров
bool is_overlap(const vec2 a_local_pos, const Collider& a_collider,
                const vec2 b_local_pos, const Collider& b_collider);

// Определяет наложение коллайдеров
bool is_overlap(const CObject& a, const CObject& b);

// Определяет наложение коллайдеров
bool is_overlap(const CObject& a, const vec2 b_local_pos, const Collider& b_collider);
