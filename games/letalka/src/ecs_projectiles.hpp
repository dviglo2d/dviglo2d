#pragma once

#include <glm/glm.hpp>

using namespace glm;


// ===================== Компоненты =====================

// Снаряд врага
struct CEnemyProjectileMarker
{
};

// Лазер игрока летит вверх
struct CPlayerLaserMarker
{
};

// Лазер врага летит вниз
struct CEnemyLaserMarker
{
};

// Плазма врага испускается в сторону игрока
struct CEnemyPlasmaMarker
{
};

// ===================== Системы =====================

void s_draw_player_lasers();
void s_draw_enemy_lasers();
void s_draw_enemy_plasmas();

// ===================== Утилиты =====================

// Лазер игрока летит вверх, а лазер врага летит вниз.
// muzzle_pos - экранная координата дула
void create_laser(const vec2 screen_muzzle_pos, bool enemy);

void create_plasma(const vec2 screen_muzzle_pos);
