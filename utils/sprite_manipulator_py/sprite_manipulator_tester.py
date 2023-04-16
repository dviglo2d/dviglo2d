#!/usr/bin/env python3

from sprite_manipulator import *

import os
import skimage as ski


if __name__ == '__main__':
    script_dir: str = os.path.dirname(os.path.abspath(__file__)) + os.path.sep # Папка, в которой находится скрипт
    src_atlas_path: str = script_dir + 'input_atlas.png'
    src_tile_width: int = 16 # Ширина тайла в исходном аталасе
    src_tile_height: int = 32
    border_size: int = 2 # На сколько будет расширен каждый тайл
    result_atlas_path: str = script_dir + 'result_atlas.png'
    result_xml_path: str = script_dir + 'result_atlas.xml'

    # Загружаем текстурный атлас
    input_image: Image = ski.io.imread(src_atlas_path)

    # Делим атлас на тайлы
    tiles: ImageArray2D = split_image(input_image, src_tile_width, src_tile_height)

    # Расширяем каждый тайл
    for index_y in range(tiles.shape[0]): # Цикл от 0 до num_tiles_y - 1
        for index_x in range(tiles.shape[1]):
            tiles[index_y, index_x] = expand_image(tiles[index_y, index_x], border_size)

    # Объединяем тайлы в новый атлас
    result_atlas, result_xml_doc = merge_tiles(tiles, border_size)
    ski.io.imsave(result_atlas_path, result_atlas)
    write_xml(result_xml_doc, result_xml_path)
