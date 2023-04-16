#include "ecs_weapons.hpp"

#include "ecs_common.hpp"
#include "ecs_enemies.hpp"
#include "ecs_player.hpp"
#include "ecs_projectiles.hpp"
#include "global.hpp"

#include <dviglo/main/timer.hpp>


// Спрайт целиком на экране
static bool is_inside_screen(const CObject& obj)
{
    vec2 half_size = obj.uv.size / 2.f;

    if (obj.pos.x - half_size.x < 0
        || obj.pos.y - half_size.y < 0
        || obj.pos.x + half_size.x >(f32)fbo_size.x
        || obj.pos.y + half_size.y >(f32)fbo_size.y)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void s_player_shoot(i64 ns)
{
    registry& reg = *GLOBAL->reg();
    entity player_ent = get_player();
    CObject& player_obj = reg.get<CObject>(player_ent);
    CGuns& player_guns = reg.get<CGuns>(player_ent);
    SDL_MouseButtonFlags mouse_state = SDL_GetMouseState(nullptr, nullptr);

    // Для каждой лазерной пушки игрока
    for (Gun& gun : player_guns.guns)
    {
        if (gun.shoot_delay > 0)
            gun.shoot_delay -= ns;

        if (gun.shoot_delay <= 0 && mouse_state & SDL_BUTTON_LMASK)
        {
            vec2 screen_muzzle_pos = player_obj.pos + gun.muzzle_pos;
            create_laser(screen_muzzle_pos, false);
            gun.shoot_delay = ns_per_s / 2;
        }
    }
}

void s_fighters_shoot(i64 ns)
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CObject, CGuns, CFighterMarker>(exclude<CDestroyedMarker>);
    for (auto [fighter_ent, fighter_obj, fighter_guns] : view.each())
    {
        // Начнёт стрелять после вылета на экран
        if (is_inside_screen(fighter_obj))
        {
            for (Gun& gun : fighter_guns.guns)
            {
                if (gun.shoot_delay > 0)
                    gun.shoot_delay -= ns;

                if (gun.shoot_delay <= 0)
                {
                    vec2 screen_muzzle_pos = fighter_obj.pos + gun.muzzle_pos;
                    create_laser(screen_muzzle_pos, true);
                    gun.shoot_delay = ns_per_s * 3 / 2;
                }
            }
        }
    }
}

void s_gunships_shoot(i64 ns)
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CObject, CGuns, CGunshipMarker>(exclude<CDestroyedMarker>);
    for (auto [gunship_ent, gunship_obj, gunship_guns] : view.each())
    {
        // Начнёт стрелять после вылета на экран
        if (is_inside_screen(gunship_obj))
        {
            for (Gun& gun : gunship_guns.guns)
            {
                if (gun.shoot_delay > 0)
                    gun.shoot_delay -= ns;

                if (gun.shoot_delay <= 0)
                {
                    vec2 screen_muzzle_pos = gunship_obj.pos + gun.muzzle_pos;
                    create_plasma(screen_muzzle_pos);
                    gun.shoot_delay = ns_per_s * 2;
                }
            }
        }
    }
}
