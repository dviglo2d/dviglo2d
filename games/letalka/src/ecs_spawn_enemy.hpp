#pragma once

#include <dv_primitive_types.hpp>

using namespace dviglo;


// ===================== Компоненты =====================

// Задержка перед созданием очередной пачки врагов
struct CSpawnEnemyDelay
{
    i64 value = 0;
};

// ===================== Системы =====================

// Создаёт очередную пачку врагов, если прошла задержка
void s_spawn_enemy(i64 ns);

// ===================== Утилиты =====================

void create_enemy_spawner();
void reset_enemy_spawner();
