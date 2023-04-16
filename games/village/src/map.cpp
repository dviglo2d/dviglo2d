#include "map.hpp"

#include <dviglo/math/random.hpp>

using namespace dviglo;


static Random rnd;

Map::Map(ivec2 size)
    : size_(size)
{
    tiles_.resize(size.y * size.x);

    for (i32 y = 0; y < size_.y; ++y)
    {
        for (i32 x = 0; x < size_.x; ++x)
        {
            Tile& tile = (*this)[y, x];

            i32 type = rnd.generate(0, 99);
            if (type < 30) // Вероятность травы - 30%
                tile.type = TileType::grass;
            else
                tile.type = TileType::sand;

            // В последнем ряду кочки не создаём - плохо смотрятся
            if (y != size_.y - 1)
            {
                i32 hummock = rnd.generate(0, 99); // Есть ли на тайле кочка
                if (hummock < 15) // Вероятность наличия кочки - 15%
                    tile.hummock = true;
            }
        }
    }
}

Map::Map(i32 size_x, i32 size_y)
    : Map(ivec2(size_x, size_y))
{
}

Tile& Map::operator [](i32 y, i32 x)
{
    assert(y >= 0 && y < size_.y && x >= 0 && x < size_.x);

    return tiles_[y * size_.x + x];
}
