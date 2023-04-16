#include "ecs_enemies.hpp"

#include "ecs_player.hpp"
#include "global.hpp"

#include <dviglo/main/timer.hpp>


void s_update_drone_velocities(i64 ns)
{
    registry& reg = *GLOBAL->reg();

    auto view = reg.view<CObject, CDrone, CVelocity>(exclude<CDestroyedMarker>);

    for (auto [ent, obj, drone, velocity] : view.each())
    {
        drone.lifetime += ns;

        constexpr f32 drone_speed = 100.f;

        // Три секунды дрон наводится на игрока по горизонтали
        if (drone.lifetime < ns_per_s * 3)
        {
            entity player_ent = get_player();
            CObject& player_obj = reg.get<CObject>(player_ent);

            f32 drone_center_x = obj.pos.x;
            f32 player_center_x = player_obj.pos.x;

            if (abs(drone_center_x - player_center_x) < 1.f)
                velocity.value = vec2(0.f, 0.f); // Чтобы дрон не дрожал на месте
            else if(drone_center_x < player_center_x)
                velocity.value = vec2(drone_speed, 0.f);
            else
                velocity.value = vec2(-drone_speed, 0.f);
        }
        else // А потом движется вниз
        {
            velocity.value = vec2(0.f, drone_speed);
        }
    }
}

void s_draw_enemies()
{
    registry& reg = *GLOBAL->reg();
    SpriteBatch& sprite_batch = *GLOBAL->sprite_batch();

    auto view = reg.view<CObject, CEnemy>();
    for (entity ent : view)
    {
        CObject& obj = view.get<CObject>(ent);
        sprite_batch.draw_sprite(GLOBAL->spritesheet(), obj.pos - obj.uv.size * 0.5f, &obj.uv);
    }
}

void s_draw_enemy_hitboxes()
{
    registry& reg = *GLOBAL->reg();
    SpriteBatch& sprite_batch = *GLOBAL->sprite_batch();

    auto view = reg.view<CObject, CEnemy>();
    for (auto [ent, obj, enemy] : view.each())
    {
        sprite_batch.set_shape_color(0x60FFFFFF);
        sprite_batch.draw_rect({obj.pos + enemy.hitbox.pos - enemy.hitbox.half_size,
                                enemy.hitbox.half_size * 2.f});
    }
}
