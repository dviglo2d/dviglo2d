#include "map.hpp"

#include <dv_random.hpp>

using namespace dviglo;


static Random rnd;

Map::Map()
{
    size_ = ivec2(10, 9);

    tiles_.resize(size_.y * size_.x);

    // Заполняем карту травой
    for (i32 y = 0; y < size_.y; ++y)
    {
        for (i32 x = 0; x < size_.x; ++x)
        {
            Tile& tile = (*this)[y, x];
            tile.type = TileType::grass;
        }
    }

    // Обновляем uv тайлов. Считаем в целых координатах, чтобы не было ошибок округления
    for (i32 y = 0; y < size_.y; ++y)
    {
        for (i32 x = 0; x < size_.x; ++x)
        {
            Tile& tile = (*this)[y, x];

            if (tile.type == TileType::grass)
            {
                IntRect tile_uv(1 * Tile::size, 1 * Tile::size, Tile::size, Tile::size);

                if (x == 0) // Левый тайл карты
                    tile_uv.pos.x -= Tile::size;
                else if (x == size_.x - 1) // Правый тайл карты
                    tile_uv.pos.x += Tile::size;

                if (y == 0) // Верхний тайл карты
                    tile_uv.pos.y -= Tile::size;
                else if (y == size_.y - 1) // Нижний тайл карты
                    tile_uv.pos.y += Tile::size;

                tile.uv = tile_uv;
            }
        }
    }

    // Расставляем скалы
    (*this)[2, 3].obstacle = true;
    (*this)[4, 3].obstacle = true;
    (*this)[5, 3].obstacle = true;
    (*this)[5, 4].obstacle = true;
    (*this)[5, 6].obstacle = true;

    // Обновляем uv скал. Считаем в целых координатах, чтобы не было ошибок округления
    for (i32 y = 0; y < size_.y; ++y)
    {
        for (i32 x = 0; x < size_.x; ++x)
        {
            Tile& tile = (*this)[y, x];

            if (tile.obstacle)
            {
                // TODO: Искать скалы вокруг тайла и менять спрайт скалы
                tile.obstacle_uv = Rect(3 * 64.f, 4 * 64.f, 64.f, 128.f);
            }
        }
    }
}

Tile& Map::operator [](ivec2 xy)
{
    assert(xy.y >= 0 && xy.y < size_.y && xy.x >= 0 && xy.x < size_.x);

    return tiles_[xy.y * size_.x + xy.x];
}

Tile& Map::operator [](i32 y, i32 x)
{
    assert(y >= 0 && y < size_.y && x >= 0 && x < size_.x);

    return tiles_[y * size_.x + x];
}
