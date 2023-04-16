#pragma once

#include <dviglo/common/primitive_types.hpp>
#include <glm/glm.hpp>
#include <vector>

using namespace dviglo;
using namespace glm;
using namespace std;


// ===================== Утилиты =====================

struct Gun
{
    // Остаток времени до следующего выстрела (в наносекундах)
    u64 shoot_delay = 0;

    // Координаты дула относительно центра спрайта корабля
    vec2 muzzle_pos{0.f, 0.f};
};

// ===================== Компоненты =====================

// Чтобы прикрепить к кораблю несколько компонентов одного типа, помещаем их в компонент-массив
struct CGuns
{
    // Автор entt рекомендует использовать std::array, но не будем усложнять код
    // https://stackoverflow.com/questions/57235844/how-to-handle-dynamic-hierarchical-entities-in-ecs
    vector<Gun> guns;
};

// ===================== Системы =====================

void s_player_shoot(u64 ns);
void s_fighters_shoot(u64 ns);
void s_gunships_shoot(u64 ns);
