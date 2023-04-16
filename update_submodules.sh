#!/bin/sh

# Обновляет подмодули до последних версий

set -x # Включаем эхо

this_dir=$(dirname "$0")

# Переключаем ветку SDL_mixer на новую
sm_path=engine/third_party/external/sdl_mixer/repo
sm_branch=dv_2025_09_04
git -C "$this_dir" config -f .gitmodules submodule.$sm_path.branch $sm_branch
git -C "$this_dir" submodule sync

# Не используем параметр --recursive, так как апстрим https://github.com/freetype/freetype
# использует не последнюю версию подмодуля https://github.com/nyorain/dlg
git -C "$this_dir" submodule update --remote
