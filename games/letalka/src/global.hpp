#pragma once

#include <dv_subsystem_index.hpp>
#include <dviglo/graphics/sprite_batch.hpp>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

using namespace dviglo;
using namespace entt;
using namespace glm;
using namespace std;


class Global final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static Global* instance_ = nullptr;

    unique_ptr<SpriteBatch> sprite_batch_;
    unique_ptr<SpriteFont> r_20_font_;
    shared_ptr<Texture> spritesheet_;
    unique_ptr<registry> reg_;

public:
    static Global* instance() { return instance_; }

    // Неуязвимость корабля игрока
    bool god_mode = false;

    // Рисовать ли коллайдеры объектов
    bool debug_draw = false;

    SpriteBatch* sprite_batch() const { return sprite_batch_.get(); }
    SpriteFont* r_20_font() const { return r_20_font_.get(); }
    Texture* spritesheet() const { return spritesheet_.get(); }
    registry* reg() const { return reg_.get(); }

    Global();
    ~Global() final;
};

#define GLOBAL (Global::instance())

inline constexpr ivec2 fbo_size{900, 700};
