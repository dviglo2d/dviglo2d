#!/bin/sh

set -x # Включаем эхо

this_dir=$(dirname "$0")

remove_submodule()
{
    git -C "$this_dir" submodule deinit -f $1
    git -C "$this_dir" rm -f $1
    rm -rf "$this_dir/.git/modules/$1"
}

remove_submodule engine/third_party/external/freetype/repo
remove_submodule engine/third_party/external/glm/repo
remove_submodule engine/third_party/external/miniz/repo
remove_submodule engine/third_party/external/pugixml/repo
remove_submodule engine/third_party/external/sdl/repo
remove_submodule engine/third_party/external/sdl_mixer/repo
remove_submodule engine/third_party/external/stb/repo
remove_submodule third_party/external/entt/repo
remove_submodule third_party/external/imgui/repo
