#pragma once

#include <dviglo/common/primitive_types.hpp>
#include <dviglo/graphics/sprite_batch.hpp>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

using namespace dviglo;
using namespace entt;
using namespace glm;
using namespace std;


class Global
{
private:
    // Инициализируется в конструкторе
    inline static Global* instance_ = nullptr;

    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> r_20_font_;
    Texture* spritesheet_;
    unique_ptr<registry> reg_;

public:
    static Global* instance() { return instance_; }

    // Неуязвимость корабля игрока
    bool god_mode = false;

    // Рисовать ли коллайдеры объектов
    bool debug_draw = false;

    SpriteBatch* sprite_batch() const { return sprite_batch_.get(); }
    SpriteFont* r_20_font() const { return r_20_font_.get(); }
    Texture* spritesheet() const { return spritesheet_; }
    registry* reg() const { return reg_.get(); }

    Global();
    ~Global();
};

#define GLOBAL (Global::instance())

inline constexpr u64 ns_per_second = 1000000000;
inline constexpr ivec2 fbo_size{900, 700};

// Безопасное вычитание, чтобы не было переполнения
inline constexpr void decrease_delay(u64& delay, const u64 time_step)
{
    if (delay >= time_step)
        delay -= time_step;
    else
        delay = 0;
}
