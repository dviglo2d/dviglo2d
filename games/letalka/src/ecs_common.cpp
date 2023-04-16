#include "ecs_common.hpp"

#include "global.hpp"


void s_apply_velocities(u64 ns)
{
    registry& reg = *GLOBAL->reg();

    auto view = reg.view<CObject, CVelocity>(exclude<CDestroyedMarker>);
    for (auto [ent, obj, vel] : view.each())
    {
        // Двигаем объект
        f32 seconds = (f32)ns / ns_per_second;
        obj.pos += vel.value * seconds;

        // Вражеские корабли создаются за границей экрана, поэтому удаляем только те объекты,
        // которые находятся далеко от границы
        const f32 safe_size = 500.f;

        if (obj.pos.x < -safe_size || obj.pos.x > (f32)fbo_size.x + safe_size ||
            obj.pos.y < -safe_size || obj.pos.y > (f32)fbo_size.y + safe_size)
        {
            reg.emplace<CDestroyedMarker>(ent);
        }
    }
}

void s_draw_colliders()
{
    registry& reg = *GLOBAL->reg();
    SpriteBatch& sprite_batch = *GLOBAL->sprite_batch();

    auto view = reg.view<CObject>();
    for (auto [ent, obj] : view.each())
    {
        sprite_batch.set_shape_color(0x60FFFFFF);
        sprite_batch.draw_rect({obj.pos + obj.collider.pos - obj.collider.half_size,
                                obj.collider.half_size * 2.f});
    }
}

bool is_overlap(const vec2 a_local_pos, const Collider& a_collider,
                const vec2 b_local_pos, const Collider& b_collider)
{
    // Экранные координаты центров коллайдеров
    vec2 a_pos = a_local_pos + a_collider.pos;
    vec2 b_pos = b_local_pos + b_collider.pos;

    // Касание не является перекрытием
    if (a_pos.x + a_collider.half_size.x <= b_pos.x - b_collider.half_size.x     // a слева от b
        || b_pos.x + b_collider.half_size.x <= a_pos.x - a_collider.half_size.x  // b слева от a
        || a_pos.y + a_collider.half_size.y <= b_pos.y - b_collider.half_size.y  // a выше b
        || b_pos.y + b_collider.half_size.y <= a_pos.y - a_collider.half_size.y) // b выше a
    {
        return false;
    }

    return true;
}

bool is_overlap(const CObject& a, const CObject& b)
{
    return is_overlap(a.pos, a.collider, b.pos, b.collider);
}

bool is_overlap(const CObject& a, const vec2 b_local_pos, const Collider& b_collider)
{
    return is_overlap(a.pos, a.collider, b_local_pos, b_collider);
}
