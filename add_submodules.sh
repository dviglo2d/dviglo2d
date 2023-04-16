#!/bin/sh

set -x # Включаем эхо

this_dir=$(dirname "$0")

add_submodule()
{
    git -C "$this_dir" submodule add $2 $1
    git -C "$this_dir" config -f .gitmodules submodule.$1.shallow true
}

add_submodule engine/third_party/external/freetype/repo https://github.com/freetype/freetype
add_submodule engine/third_party/external/glm/repo https://github.com/g-truc/glm
add_submodule engine/third_party/external/miniz/repo https://github.com/richgel999/miniz
add_submodule engine/third_party/external/pugixml/repo https://github.com/zeux/pugixml
add_submodule engine/third_party/external/sdl/repo https://github.com/libsdl-org/SDL
add_submodule engine/third_party/external/sdl_mixer/repo https://github.com/libsdl-org/SDL_mixer
add_submodule engine/third_party/external/stb/repo https://github.com/nothings/stb
add_submodule third_party/external/entt/repo https://github.com/skypjack/entt
add_submodule third_party/external/imgui/repo https://github.com/ocornut/imgui

git -C "$this_dir" submodule update --init --recursive --depth 1
