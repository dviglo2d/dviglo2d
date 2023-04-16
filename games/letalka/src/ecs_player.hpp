#pragma once

#include <dv_primitive_types.hpp>
#include <entt/entt.hpp>
#include <SDL3/SDL.h>

using namespace dviglo;
using namespace entt;


// ===================== Компоненты =====================

struct CPlayer
{
    // Число убитых врагов
    i32 score = 0;
};

// ===================== Утилиты =====================

// Создаёт новый корабль игрока
void create_player();

// Игроков может быть несколько, но всегда только один живой
entity get_player();

// ===================== Системы =====================

// Корабль игрока реагирует на движение мыши
void s_player_on_mouse_motion(const SDL_MouseMotionEvent& event_data);

void s_draw_player();
void s_draw_score();
