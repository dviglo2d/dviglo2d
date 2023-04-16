#!/bin/sh

set -x # Включаем эхо

this_dir=$(dirname "$0")

sm_path=engine/third_party/external/freetype/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/freetype $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=engine/third_party/external/glm/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/glm $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=engine/third_party/external/miniz/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/miniz $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=engine/third_party/external/pugixml/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/pugixml $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=engine/third_party/external/sdl/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/sdl $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=engine/third_party/external/sdl_mixer/repo
sm_branch=dv_2024_11_06
# -b branch и --depth 1 вместе не работают
git -C "$this_dir" submodule add -b $sm_branch https://github.com/dviglo2d/sdl_mixer $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=engine/third_party/external/stb/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/stb $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=third_party/external/entt/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/entt $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

sm_path=third_party/external/imgui/repo
git -C "$this_dir" submodule add --depth 1 https://github.com/dviglo2d/imgui $sm_path
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.shallow true

git -C "$this_dir" submodule update --init --recursive
