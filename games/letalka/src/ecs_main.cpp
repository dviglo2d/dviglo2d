#include "ecs_main.hpp"


void restart_game()
{
    registry& reg = *GLOBAL->reg();

    reset_enemy_spawner();

    // Очищаем мир от врагов
    {
        auto view = reg.view<CEnemy>(exclude<CDestroyedMarker>);
        for (entity ent : view)
            reg.emplace<CDestroyedMarker>(ent);
    }

    // Очищаем мир от проджектайлов врагов
    {
        auto view = reg.view<CEnemyProjectileMarker>(exclude<CDestroyedMarker>);
        for (entity ent : view)
            reg.emplace<CDestroyedMarker>(ent);
    }

    // Очищаем мир от проджектайлов игрока
    {
        auto view = reg.view<CPlayerLaserMarker>(exclude<CDestroyedMarker>);
        for (entity ent : view)
            reg.emplace<CDestroyedMarker>(ent);
    }

    // Пересоздаём персонажа
    {
        entity ent = get_player();
        reg.emplace<CDestroyedMarker>(ent);
        create_player();
    }
}

void s_remove_destroyed()
{
    registry& reg = *GLOBAL->reg();
    for (entity ent : reg.view<CDestroyedMarker>())
        reg.destroy(ent);
}

void s_check_collisions()
{
    registry& reg = *GLOBAL->reg();
    entity player_ent = get_player();
    CObject& player_obj = reg.get<CObject>(player_ent);

    // Ищем столкновения снарядов игрока со снарядами врагов
    {
        auto enemy_proj_view = reg.view<CObject, CEnemyProjectileMarker>(exclude<CDestroyedMarker>);
        for (entity enemy_proj_ent : enemy_proj_view)
        {
            CObject& enemy_proj_obj = enemy_proj_view.get<CObject>(enemy_proj_ent);

            auto player_laser_view = reg.view<CObject, CPlayerLaserMarker>(exclude<CDestroyedMarker>);
            for (entity player_laser_ent : player_laser_view)
            {
                CObject& player_laser_obj = player_laser_view.get<CObject>(player_laser_ent);

                if (is_overlap(player_laser_obj, enemy_proj_obj))
                {
                    reg.emplace<CDestroyedMarker>(player_laser_ent);
                    reg.emplace<CDestroyedMarker>(enemy_proj_ent);
                    break;
                }
            }
        }
    }

    // Ищем столкновения снарядов игрока и кораблей противников
    {
        auto enemy_view = reg.view<CObject, CEnemy>(exclude<CDestroyedMarker>);
        for (entity enemy_ent : enemy_view)
        {
            CObject& enemy_obj = enemy_view.get<CObject>(enemy_ent);
            CEnemy& enemy_enemy = enemy_view.get<CEnemy>(enemy_ent);

            auto player_laser_view = reg.view<CObject, CPlayerLaserMarker>(exclude<CDestroyedMarker>);
            for (entity player_laser_ent : player_laser_view)
            {
                CObject& player_laser_obj = player_laser_view.get<CObject>(player_laser_ent);

                // Используем хитбокс врага, который больше коллайдера
                if (is_overlap(player_laser_obj, enemy_obj.pos, enemy_enemy.hitbox))
                {
                    CPlayer& player = reg.get<CPlayer>(player_ent);
                    reg.emplace<CDestroyedMarker>(player_laser_ent);
                    reg.emplace<CDestroyedMarker>(enemy_ent);
                    ++player.score;
                    break;
                }
            }
        }
    }

    if (!GLOBAL->god_mode)
    {
        // Ищем столкновения снарядов противника и корабля игрока
        auto enemy_proj_view = reg.view<CObject, CEnemyProjectileMarker>(exclude<CDestroyedMarker>);
        for (entity enemy_proj_ent : enemy_proj_view)
        {
            CObject& enemy_proj_obj = enemy_proj_view.get<CObject>(enemy_proj_ent);

            if (is_overlap(player_obj, enemy_proj_obj))
                restart_game();
        }

        // Ищем столкновения корабля игрока с кораблями противника
        auto enemy_view = reg.view<CObject, CEnemy>(exclude<CDestroyedMarker>);
        for (entity enemy_ent : enemy_view)
        {
            CObject& enemy_obj = enemy_view.get<CObject>(enemy_ent);

            if (is_overlap(player_obj, enemy_obj))
                restart_game();
        }
    }
}
