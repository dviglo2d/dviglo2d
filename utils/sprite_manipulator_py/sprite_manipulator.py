from typing import TypeAlias
import numpy as np
import xml.etree.ElementTree as et


# Псевдоним для изображения, которое на самом деле просто массив NumPy
Image: TypeAlias = np.ndarray[tuple[int, ...], np.dtype[np.uint8]]

# Псевдоним для двумерного массива изображений
ImageArray2D: TypeAlias = np.ndarray[tuple[int, int], np.dtype[np.object_]]


# Расширяет изображение на border_size в каждую сторону, копируя крайние пиксели исходного изображения
def expand_image(src_image: Image, border_size: int) -> Image:
    assert(src_image.ndim == 3) # Изображение цветное
    assert(src_image.size) # Изображение не пустое
    assert(src_image.dtype == np.uint8) # Массив содержит байты
    assert(border_size >= 0)

    pad_width = (
        (border_size, border_size), # Верх и низ
        (border_size, border_size), # Лево и право
        (0, 0)                      # Число каналов не меняем
    )

    return np.pad(src_image, pad_width, mode='edge')


# Делит изображение на тайлы. Возвращает двумерный массив изображений
def split_image(image: Image, tile_size_x: int, tile_size_y: int) -> ImageArray2D:
    assert(image.ndim == 3) # Изображение цветное
    assert(image.size) # Изображение не пустое
    assert(image.dtype == np.uint8) # Массив содержит байты
    assert(tile_size_x > 0 and tile_size_y > 0)
    assert(image.shape[0] % tile_size_y == 0) # Высота делится без остатка
    assert(image.shape[1] % tile_size_x == 0) # Ширина делится без остатка

    num_tiles_y: int = image.shape[0] // tile_size_y # Целочисленное деление
    num_tiles_x: int = image.shape[1] // tile_size_x

    # Создаём двумерный массив для хранения тайлов
    ret: ImageArray2D = np.empty((num_tiles_y, num_tiles_x), dtype=np.object_)

    for index_y in range(num_tiles_y): # Цикл от 0 до num_tiles_y - 1
        for index_x in range(num_tiles_x):
            start_y: int = index_y * tile_size_y
            end_y: int = start_y + tile_size_y
            start_x: int = index_x * tile_size_x
            end_x: int = start_x + tile_size_x
            ret[index_y, index_x] = image[start_y:end_y, start_x:end_x]

    return ret


# Округляет вверх до ближайшей степени двойки
def ceil_power_of_2(n: int) -> int:
    assert(n >= 1)

    ret: int = 1
    while ret < n:
        ret *= 2 # 1, 2, 4, 8, 16, 32, ...

    return ret


# Вставляет одно изображение в другое
def paste(dest: Image, src: Image, dest_pos_x: int, dest_pos_y: int):
    assert(dest.ndim == 3 and src.ndim == 3) # Изображения цветные
    assert(dest.dtype == np.uint8 and src.dtype == np.uint8) # Массивы содержат байты
    assert(dest.shape[2] == src.shape[2]) # Число каналов одинаково
    assert(dest_pos_x >= 0 and dest_pos_y >= 0)

    if (dest.size == 0 or src.size == 0): # Одно из изображений пустое
        return

    dest[dest_pos_y:dest_pos_y+src.shape[0], dest_pos_x:dest_pos_x+src.shape[1]] = src


# Объединяет двумерный массив тайлов в атлас. Все тайлы должны быть одинакового размера.
# Возвращает изображение и xml-документ
def merge_tiles(tiles: ImageArray2D, border_size: int = 0) -> tuple[Image, et.ElementTree]:
    assert(tiles.ndim == 2) # Массив двумерный
    assert(tiles.size) # Массив не пустой
    assert(tiles[0, 0].ndim == 3) # Изображение цветное
    assert(tiles[0, 0].dtype == np.uint8) # Массив содержит байты

    # Все тайлы одинакового размера, поэтому используем размер первого изображения
    tile_size_y, tile_size_x, channels = tiles[0, 0].shape

    atlas_size_y: int = ceil_power_of_2(tiles.shape[0] * tile_size_y)
    atlas_size_x: int = ceil_power_of_2(tiles.shape[1] * tile_size_x)
    atlas: Image = np.zeros((atlas_size_y, atlas_size_x, channels), tiles[0, 0].dtype)

    # Создаём корневой элемент xml
    root: et.Element = et.Element('tiles')

    for index_y in range(tiles.shape[0]): # Цикл от 0 до num_tiles_y - 1
        for index_x in range(tiles.shape[1]):
            assert(tiles[index_y, index_x].shape == tiles[0, 0].shape) # Все тайлы одинакового размера
            paste(atlas, tiles[index_y, index_x], index_x * tile_size_x, index_y * tile_size_y)
            tile_element: et.Element = et.SubElement(root, 'tile')
            tile_element.set('x', str(index_x * tile_size_x + border_size))
            tile_element.set('y', str(index_y * tile_size_y + border_size))
            tile_element.set('width', str(tile_size_x - border_size * 2))
            tile_element.set('height', str(tile_size_y - border_size * 2))

    # Создаём xml-документ
    tree: et.ElementTree = et.ElementTree(root)
    et.indent(tree, space='    ')

    return (atlas, tree)


# Сохраняет xml-файл с переводом строки в конце
def write_xml(xml_doc: et.ElementTree, filename: str):
    # Вместо xml_doc.write(filename) открываем файл как двоичный, чтобы на любой платформе в качестве разделителя был \n
    with open(filename, 'wb') as f:
        xml_doc.write(f, encoding='utf-8')
        f.write(b'\n')
