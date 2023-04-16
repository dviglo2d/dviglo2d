#include "ecs_spawn_enemy.hpp"

#include "ecs_common.hpp"
#include "ecs_enemies.hpp"
#include "ecs_weapons.hpp"
#include "global.hpp"

#include <random>

using namespace std;


class Random
{
private:
    mt19937 generator_;

public:
    Random()
    {
        // Используем random_device только для генерации seed, так как он медленный
        random_device device;
        u32 seed = device();

        // А дальше будем использовать generator_
        generator_.seed(seed);
    }

    // Генерирует случайное число из диапазона [min, max] включительно
    i32 generate(i32 min, i32 max)
    {
        uniform_int_distribution<i32> dist(min, max);
        return dist(generator_);
    }
};

static Random rnd;

static const Rect drone_uv({108.f, 2.f}, {64.f, 64.f});
static const Rect fighter_uv({182.f, 6.f}, {74.f, 64.f});
static const Rect gunship_uv({269.f, 7.f}, {64.f, 64.f});

static void create_drone(vec2 pos)
{
    registry& reg = *GLOBAL->reg();
    Collider collider({0.f, 0.f}, {25.f, 25.f});
    Collider hitbox({0.f, 0.f}, {30, 30});

    entity ent = reg.create();
    reg.emplace<CObject>(ent, pos, drone_uv, collider);
    reg.emplace<CEnemy>(ent, hitbox);
    reg.emplace<CDrone>(ent, u64(0));
    reg.emplace<CVelocity>(ent, vec2(0.f, 0.f));
}

static void spawn_drones()
{
    f32 y = 130.f;

    // Дрон слева
    f32 x = -drone_uv.size.y / 2;
    create_drone({x, y});

    // Дрон справа
    x = (f32)fbo_size.x + drone_uv.size.y / 2;
    create_drone({x, y});
}

static void create_fighter(vec2 pos)
{
    registry& reg = *GLOBAL->reg();
    Collider collider({0.f, -6.f}, {16.f, 25.f});
    Collider hitbox({0.f, 0.f}, {37, 32});

    entity ent = reg.create();
    reg.emplace<CObject>(ent, pos, fighter_uv, collider);
    reg.emplace<CEnemy>(ent, hitbox);
    reg.emplace<CFighterMarker>(ent);
    reg.emplace<CVelocity>(ent, vec2(0.f, 200.f));

    vector<Gun> guns;
    guns.emplace_back(u64(0), vec2(0.f, 32.f));
    reg.emplace<CGuns>(ent, guns);
}

static void spawn_fighters()
{
    // Истребитель за верхней границей экрана
    f32 x = (f32)rnd.generate(i32(fighter_uv.size.x / 2), i32(fbo_size.x - fighter_uv.size.x / 2));
    f32 y = -fighter_uv.size.y / 2.f;
    create_fighter({x, y});

    // Второй истребитель выше первого, поэтому вылетит из-за края экрана позже
    x = (f32)rnd.generate(i32(fighter_uv.size.x / 2), i32(fbo_size.x - fighter_uv.size.x / 2));
    y = -fighter_uv.size.y * 2.5f;
    create_fighter({x, y});
}

static void create_gunship(vec2 pos)
{
    registry& reg = *GLOBAL->reg();
    Collider collider({0.f, 0.f}, {25.f, 25.f});
    Collider hitbox({0.f, 0.f}, {30, 30});

    entity ent = reg.create();
    reg.emplace<CObject>(ent, pos, gunship_uv, collider);
    reg.emplace<CEnemy>(ent, hitbox);
    reg.emplace<CGunshipMarker>(ent);

    if (pos.x < fbo_size.x / 2) // Появляется слева - движемся вправо
        reg.emplace<CVelocity>(ent, vec2(300.f, 0.f));
    else // Движемся влево
        reg.emplace<CVelocity>(ent, vec2(-300.f, 0.f));

    vector<Gun> guns;
    guns.emplace_back(u64(0), vec2(0.f, 0.f));
    reg.emplace<CGuns>(ent, guns);
}

static void spawn_gunships()
{
    // Ганшип слева
    f32 x = -(f32)gunship_uv.size.x / 2;
    f32 y = (f32)rnd.generate(100, 600);
    create_gunship({x, y});

    // Ганшип справа
    x = (f32)fbo_size.x + gunship_uv.size.x / 2;
    y = (f32)rnd.generate(100, 600);
    create_gunship({x, y});
}

void create_enemy_spawner()
{
    registry& reg = *GLOBAL->reg();
    entity ent = reg.create();
    // Задержка перед первой пачкой врагов - 1 секунда
    reg.emplace<CSpawnEnemyDelay>(ent, u64(ns_per_second * 1));
}

void reset_enemy_spawner()
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CSpawnEnemyDelay>();
    entity ent = view.front(); // Компонент всегда один
    CSpawnEnemyDelay& delay = view.get<CSpawnEnemyDelay>(ent);
    // Задержка перед первой пачкой врагов - 1 секунда
    delay.value = ns_per_second * 1;
}

void s_spawn_enemy(u64 ns)
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CSpawnEnemyDelay>();
    entity ent = view.front(); // Компонент всегда один
    CSpawnEnemyDelay& delay = view.get<CSpawnEnemyDelay>(ent);

    decrease_delay(delay.value, ns);

    if (delay.value == 0)
    {
        i32 enemy_type = rnd.generate(0, 2);

        if (enemy_type == 0)
            spawn_drones();
        else if (enemy_type == 1)
            spawn_fighters();
        else // enemy_type == 2
            spawn_gunships();

        delay.value = ns_per_second * 2;
    }
}
