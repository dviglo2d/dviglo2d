#include "ecs_player.hpp"

#include "ecs_common.hpp"
#include "ecs_projectiles.hpp"
#include "ecs_weapons.hpp"
#include "global.hpp"


void create_player()
{
    vec2 pos(fbo_size.x / 2, fbo_size.y - 50.f);
    Rect uv({0.f, 0.f}, {100.f, 100.f});
    Collider collider({0.f, 10.f}, {40.f, 30.f});

    registry& reg = *GLOBAL->reg();
    entity ent = reg.create();
    reg.emplace<CPlayer>(ent);
    reg.emplace<CObject>(ent, pos, uv, collider);

    // Добавляем два лазера
    vector<Gun> guns;
    guns.emplace_back(i64(0), vec2(-32.f, -21.f));
    guns.emplace_back(i64(0), vec2(32.f, -21.f));
    reg.emplace<CGuns>(ent, guns);
}

entity get_player()
{
    registry& reg = *GLOBAL->reg();
    auto view = reg.view<CPlayer>(exclude<CDestroyedMarker>);
    return view.front(); // Живой игрок всегда один
}

void s_player_on_mouse_motion(const SDL_MouseMotionEvent& event_data)
{
    registry& reg = *GLOBAL->reg();
    entity ent = get_player();
    CObject& obj = reg.get<CObject>(ent);

    vec2 delta(event_data.xrel, event_data.yrel);
    obj.pos += delta;

    // Не позволяем кораблю персонажа выходить за границы экрана
    obj.pos = glm::clamp(obj.pos,
                         -obj.collider.pos + obj.collider.half_size,
                         vec2(fbo_size) - obj.collider.pos - obj.collider.half_size);
}

void s_draw_player()
{
    registry& reg = *GLOBAL->reg();
    entity ent = get_player();
    CObject& obj = reg.get<CObject>(ent);
    GLOBAL->sprite_batch()->draw_sprite(GLOBAL->spritesheet(), obj.pos - obj.uv.size * 0.5f, &obj.uv);
}

void s_draw_score()
{
    registry& reg = *GLOBAL->reg();
    SpriteBatch& sprite_batch = *GLOBAL->sprite_batch();
    entity ent = get_player();
    CPlayer& player = reg.get<CPlayer>(ent);
    StrUtf8 score_text = "Счёт: " + to_string(player.score);
    sprite_batch.draw_string(score_text, GLOBAL->r_20_font(), vec2(4.f, 1.f), 0xFF000000);
    sprite_batch.draw_string(score_text, GLOBAL->r_20_font(), vec2(3.f, 0.f), 0xFFFFFFFF);
}
