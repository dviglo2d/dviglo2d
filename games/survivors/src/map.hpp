#pragma once

#include <dviglo/math/rect.hpp>
#include <glm/glm.hpp>
#include <vector>

using namespace dviglo;
using namespace glm;
using namespace std;


enum class TileType
{
    grass, // Трава
};


struct Tile
{
    static constexpr i32 size = 64;

    TileType type = TileType::grass;
    Rect uv; // Даже тайлы одного типа могут выглядеть по разному

    bool obstacle = false; // Есть ли на тайле скала
    Rect obstacle_uv;
};


class Map
{
    vector<Tile> tiles_;
    ivec2 size_;

public:
    Map();

    ivec2 size() const { return size_; }

    Tile& operator [](ivec2 xy);

    // Доступ к вектору тайлов как к двумерному массиву
    Tile& operator [](i32 y, i32 x);
};
