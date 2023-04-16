#include "puzzle_logic.hpp"

#include <cassert>
#include <dv_random.hpp>


// Проверяет, что координаты находятся в допустимых пределах
static constexpr bool check_tile_pos(ivec2 pos)
{
    return pos.x >= 0 && pos.x <= 3 && pos.y >= 0 && pos.y <= 3;
}

PuzzleLogic::PuzzleLogic()
{
    new_game();
}

// Если просто расставить костяшки случайным образом, то может сгенерироваться
// нерешаемая последовательность. Есть методы оценки решаемости, но это сделает код менее читаемым.
// Поэтому просто двигаем дырку много раз
void PuzzleLogic::new_game()
{
    for (u8 i = 0; i <= 14; ++i)
        tiles_[i] = i + 1; // Нумерация костяшек начинается с 1

    tiles_[15] = 0; // Последняя клетка - пустая

#if 1
    vec2 hole_pos{3, 3}; // Позиция дырки

    // Двигаем дырку много раз.
    // Если двигать дырку мало раз, то верх поля может быть плохо перемешан
    for (i32 i = 0; i < 1000; ++i)
    {
        static Random rnd;
        i32 dir = rnd.generate(0, 3);

        if (dir == 0) // Двигаем дырку вверх
        {
            if (hole_pos.y == 0) // Вверху - край коробки
                continue;

            // Меняем местами костяшку и дырку
            vec2 other_pos{hole_pos.x, hole_pos.y - 1};
            swap_tiles(hole_pos, other_pos);
            hole_pos = other_pos;
        }
        else if (dir == 1) // Двигаем дырку вниз
        {
            if (hole_pos.y == 3) // Внизу - край коробки
                continue;

            // Меняем местами костяшку и дырку
            vec2 other_pos{hole_pos.x, hole_pos.y + 1};
            swap_tiles(hole_pos, other_pos);
            hole_pos = other_pos;
        }
        else if (dir == 2) // Двигаем дырку влево
        {
            if (hole_pos.x == 0) // Слева - край коробки
                continue;

            // Меняем местами костяшку и дырку
            vec2 other_pos{hole_pos.x - 1 , hole_pos.y};
            swap_tiles(hole_pos, other_pos);
            hole_pos = other_pos;
        }
        else // Двигаем дырку вправо
        {
            if (hole_pos.x == 3) // Справа - край коробки
                continue;

            // Меняем местами костяшку и дырку
            vec2 other_pos{hole_pos.x + 1 , hole_pos.y};
            swap_tiles(hole_pos, other_pos);
            hole_pos = other_pos;
        }
    }
#endif
}

void PuzzleLogic::swap_tiles(ivec2 pos1, vec2 pos2)
{
    assert(check_tile_pos(pos1) && check_tile_pos(pos2));
    u8 temp = get_tile(pos1);
    set_tile(pos1, get_tile(pos2));
    set_tile(pos2, temp);
}

void PuzzleLogic::set_tile(ivec2 pos, u8 value)
{
    assert(check_tile_pos(pos) && value <= 15);

    tiles_[pos.y * 4 + pos.x] = value;
}

u8 PuzzleLogic::get_tile(ivec2 pos) const
{
    assert(check_tile_pos(pos));

    return tiles_[pos.y * 4 + pos.x];
}

bool PuzzleLogic::move(ivec2 pos)
{
    if (!check_tile_pos(pos))
        return false;

    if (get_tile(pos) == 0) // Дырку не двигаем
        return false;

    // Ищем дырку слева
    if (pos.x > 0 && get_tile({pos.x - 1, pos.y}) == 0)
    {
        // Двигаем костяшку влево
        swap_tiles(pos, {pos.x - 1, pos.y});
        return true;
    }

    // Ищем дырку справа
    if (pos.x < 3 && get_tile({pos.x + 1, pos.y}) == 0)
    {
        // Двигаем костяшку вправо
        swap_tiles(pos, {pos.x + 1, pos.y});
        return true;
    }

    // Ищем дырку сверху
    if (pos.y > 0 && get_tile({pos.x, pos.y - 1}) == 0)
    {
        // Двигаем костяшку вверх
        swap_tiles(pos, {pos.x, pos.y - 1});
        return true;
    }

    // Ищем дырку снизу
    if (pos.y < 3 && get_tile({pos.x, pos.y + 1}) == 0)
    {
        // Двигаем костяшку вниз
        swap_tiles(pos, {pos.x, pos.y + 1});
        return true;
    }

    // Дырок рядом нет
    return false;
}

// Результат работы функции можно кэшировать, но не стал усложнять код
bool PuzzleLogic::check_win() const
{
    for (u8 i = 0; i <= 14; ++i)
    {
        if (tiles_[i] != i + 1)
            return false;
    }

    // Проверяем последнюю клетку
    return tiles_[15] == 0;
}
