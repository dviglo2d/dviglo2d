#!/bin/sh

# Обновляет подмодули до последних версий

set -x # Включаем эхо

this_dir=$(dirname "$0")

# Обновляем подмодули до последних версий, но не рекурсивно, так как авторы подмодулей ещё не обновили свои подмодули
git -C "$this_dir" submodule update --init --remote --depth 1

# Качаем подмодули подмодулей
git -C "$this_dir" submodule update --init --recursive --depth 1

read -p "Для продолжения нажмите Enter..."
