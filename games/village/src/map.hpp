#pragma once

#include <glm/glm.hpp>

#include <vector>

using namespace glm;
using namespace std;


enum class TileType
{
    grass, // Трава
    sand   // Песок
};

struct Tile
{
    TileType type = TileType::grass;
    bool hummock = false; // Спрайт с кочкой поверх тайла
};

class Map
{
    vector<Tile> tiles_;
    ivec2 size_;

public:
    Map(ivec2 size);
    Map(i32 size_x, i32 size_y);

    ivec2 size() const { return size_; }

    // Доступ к вектору тайлов как к двумерному массиву
    Tile& operator [](i32 y, i32 x);
};
