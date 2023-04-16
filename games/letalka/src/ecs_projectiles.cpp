#include "ecs_projectiles.hpp"

#include "ecs_common.hpp"
#include "ecs_player.hpp"
#include "global.hpp"


static void draw_laser(CObject& obj)
{
    // Размер лазера на экране совпадает с размером коллайдера
    Rect screen_rect(obj.pos - obj.collider.half_size, obj.collider.half_size * 2.f);

    GLOBAL->sprite_batch()->draw_sprite(GLOBAL->spritesheet(), screen_rect, &obj.uv, 0xAA00FF00);
}

void s_draw_player_lasers()
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CObject, CPlayerLaserMarker>();

    for (entity ent : view)
    {
        CObject& obj = view.get<CObject>(ent);
        draw_laser(obj);
    }
}

void s_draw_enemy_lasers()
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CObject, CEnemyLaserMarker>();

    for (entity ent : view)
    {
        CObject& obj = view.get<CObject>(ent);
        draw_laser(obj);
    }
}

void s_draw_enemy_plasmas()
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CObject, CEnemyPlasmaMarker>();

    for (entity ent : view)
    {
        CObject& obj = view.get<CObject>(ent);

        // Размер плазмы на экране совпадает с размером коллайдера
        Rect screen_rect(obj.pos - obj.collider.half_size, obj.collider.half_size * 2.f);

        GLOBAL->sprite_batch()->draw_sprite(GLOBAL->spritesheet(), screen_rect, &obj.uv, 0xAA00BBFF);
    }
}

void create_laser(const vec2 screen_muzzle_pos, bool enemy)
{
    registry& reg = *GLOBAL->reg();
    Rect uv({26.f, 114.f}, {128.f, 128.f});
    Collider collider({0.f, 0.f}, {2.f, 10.f}); // Заодно размер лазера на экране

    vec2 pos = screen_muzzle_pos;
    vec2 velocity = vec2(0.f, 300.f);

    if (enemy)
    {
        // Дуло направлено вниз
        pos.y -= collider.half_size.y;
    }
    else
    {
        // Дуло направлено вверх
        pos.y += collider.half_size.y;
        velocity.y = -velocity.y;
    }

    entity ent = reg.create();
    reg.emplace<CObject>(ent, pos, uv, collider);
    reg.emplace<CVelocity>(ent, velocity);

    if (enemy)
    {
        reg.emplace<CEnemyLaserMarker>(ent);
        reg.emplace<CEnemyProjectileMarker>(ent);
    }
    else
    {
        reg.emplace<CPlayerLaserMarker>(ent);
    }
}

void create_plasma(const vec2 screen_muzzle_pos)
{
    registry& reg = *GLOBAL->reg();
    Rect uv({26.f, 114.f}, {128.f, 128.f});
    Collider collider({0.f, 0.f}, {6.f, 6.f}); // Заодно размер плазмы на экране

    entity player_ent = get_player();
    CObject& player_obj = reg.get<CObject>(player_ent);
    vec2 dir = normalize(player_obj.pos - screen_muzzle_pos);
    vec2 velocity = dir * 100.f;

    entity ent = reg.create();
    reg.emplace<CObject>(ent, screen_muzzle_pos, uv, collider);
    reg.emplace<CEnemyPlasmaMarker>(ent);
    reg.emplace<CEnemyProjectileMarker>(ent);
    reg.emplace<CVelocity>(ent, velocity);
}
