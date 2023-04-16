#pragma once

#include "ecs_enemies.hpp"
#include "ecs_player.hpp"
#include "ecs_projectiles.hpp"
#include "ecs_spawn_enemy.hpp"
#include "ecs_weapons.hpp"
#include "global.hpp"


// ===================== Системы =====================

void s_check_collisions();

// Удаляет уничтоженные объекты из списков
void s_remove_destroyed();

// ================== Инициализация и апдейт ==================

inline void ecs_start()
{
    create_enemy_spawner();
    create_player();
}

inline void ecs_on_mouse_motion(const SDL_MouseMotionEvent& event_data)
{
    s_player_on_mouse_motion(event_data);
}

inline void ecs_update(i64 ns)
{
    s_spawn_enemy(ns);
    s_update_drone_velocities(ns);
    s_player_shoot(ns);
    s_fighters_shoot(ns);
    s_gunships_shoot(ns);
    s_apply_velocities(ns);
    s_check_collisions();
    s_remove_destroyed();
}

inline void ecs_draw()
{
    s_draw_player_lasers();
    s_draw_enemy_lasers();
    s_draw_enemies();
    s_draw_enemy_plasmas();
    s_draw_player();

    if (GLOBAL->debug_draw)
    {
        s_draw_colliders();
        // У врагов дополнительно есть хитбоксы
        s_draw_enemy_hitboxes();
    }

    s_draw_score();
}
