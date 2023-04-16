#!/bin/sh

# Обновляет подмодули до последних версий

set -x # Включаем эхо

this_dir=$(dirname "$0")

git -C "$this_dir" submodule update --remote --recursive --depth 1
