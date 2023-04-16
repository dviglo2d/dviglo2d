#pragma once

#include <array>
#include <dv_primitive_types.hpp>
#include <glm/glm.hpp>

using namespace dviglo;
using namespace glm;
using namespace std;


// Данный класс реализует игровую логику головоломки
class PuzzleLogic
{
private:
    // Массив костяшек. 0 - дырка
    array<u8, 16> tiles_;

    // Устанавливает число на костяшке
    void set_tile(ivec2 pos, u8 value);

    // Меняет местами два тайла
    void swap_tiles(ivec2 pos1, vec2 pos2);

public:
    PuzzleLogic();

    // Возвращает номер на костяшке. 0 - пустое поле
    u8 get_tile(ivec2 pos) const;

    // Заполняет и перемешивает игровое поле случайным образом
    void new_game();

    // Перемещает указанную костяшку в пустое поле.
    // Возвращает false, если рядом с костяшкой нет пустого поля
    // или если позиция выходит за пределы поля
    bool move(ivec2 pos);

    // Проверяет, что все костяшки на своих местах
    bool check_win() const;
};
