from PIL import Image
import numpy as np
import xml.etree.ElementTree as ET


# Делит изображение на тайлы
def split_image(image: Image.Image, tile_width: int, tile_height: int) -> np.ndarray:
    if tile_width <= 0:
        raise ValueError("tile_width <= 0")

    if tile_height <= 0:
        raise ValueError("tile_height <= 0")

    num_tiles_x: int = image.size[0] // tile_width
    num_tiles_y: int = image.size[1] // tile_height

    # Создаём двумерный массив для хранения тайлов
    ret = np.empty((num_tiles_y, num_tiles_x), dtype=object)

    for index_y in range(num_tiles_y):
        for index_x in range(num_tiles_x):
            box = (index_x * tile_width, index_y * tile_height, (index_x + 1) * tile_width, (index_y + 1) * tile_height)
            ret[index_y, index_x] = image.crop(box)

    return ret


# Расширяет изображение на border_size в каждую сторону, копируя крайние пиксели исходного изображения
def expand_image(src_image: Image.Image, border_size: int) -> Image.Image:
    if border_size < 0:
        raise ValueError("border_size < 0")

    src_width, src_height = src_image.size

    # Создаём новое изображение с увеличенным размером
    new_width: int = src_width + border_size * 2
    new_height: int = src_height + border_size * 2
    ret = Image.new("RGBA", (new_width, new_height))

    # Копируем исходное изображение в центр нового изображения
    ret.paste(src_image, (border_size, border_size))

    # Копируем верхние пиксели
    line = ret.crop((0, border_size, new_width, border_size + 1))
    for y in range(border_size):
        ret.paste(line, (0, y))

    # Копируем нижние пиксели
    line = ret.crop((0, src_height + border_size - 1, new_width, src_height + border_size))
    for y in range(src_height + border_size, new_height):
        ret.paste(line, (0, y))

    # Копируем левые пиксели
    column = ret.crop((border_size, 0, border_size + 1, new_height))
    for x in range(border_size):
        ret.paste(column, (x, 0))

    # Копируем правые пиксели
    column = ret.crop((src_width + border_size - 1, 0, src_width + border_size, new_height))
    for x in range(src_width + border_size, new_width):
        ret.paste(column, (x, 0))

    return ret


# Округляет вверх до ближайшей степени двойки
def ceil_power_of_2(n: int) -> int:
    if n < 1:
        raise ValueError("n < 1")

    ret: int = 1
    while ret < n:
        ret *= 2 # 1, 2, 4, 8, 16, 32, ...

    return ret


# Объединяет двумерный массив тайлов в атлас.
# Возвращает изображение и xml-документ
def join_tiles(tiles: np.ndarray, border_size: int = 0) -> tuple[Image.Image, ET.ElementTree]:
    if tiles.ndim != 2 or tiles.size == 0 : # Убеждаемся, что массив двумерный и не пустой
        raise ValueError()

    tile_width, tile_height = tiles[0, 0].size
    atlas_width = ceil_power_of_2(tiles.shape[1] * tile_width)
    atlas_height = ceil_power_of_2(tiles.shape[0] * tile_height)
    atlas = Image.new("RGBA", (atlas_width, atlas_height), (0, 0, 0, 0))

    # Создаём корневой элемент xml
    root = ET.Element("tiles")

    for index_y in range(tiles.shape[0]):
        for index_x in range(tiles.shape[1]):
            atlas.paste(tiles[index_y, index_x], (index_x * tile_width, index_y * tile_height))
            tile_element = ET.SubElement(root, "tile")
            tile_element.set("x", str(index_x * tile_width + border_size))
            tile_element.set("y", str(index_y * tile_height + border_size))
            tile_element.set("width", str(tile_width - border_size * 2))
            tile_element.set("height", str(tile_height - border_size * 2))

    # Создаём xml-документ
    tree = ET.ElementTree(root)
    ET.indent(tree, space="    ")

    return (atlas, tree)


# Сохраняет xml-файл с переводом строки в конце
def write_xml(xml_doc: ET.ElementTree, filename: str):
    with open(result_xml_path, "wb") as f:
        xml_doc.write(f, encoding="utf-8") # Использует \n в качестве разделителя в отличие от write(filename)
        f.write(b"\n")


# Пример использования
src_atlas_path: str = "input_atlas.png"
src_tile_width: int = 16 # Ширина тайла в исходном аталасе
src_tile_height: int = 32
border_size: int = 2 # На сколько будет расширен каждый тайл
result_atlas_path: str = "expanded_atlas.png"
result_xml_path: str = "expanded_atlas.xml"

# Загружаем текстурный атлас
input_image: Image.Image = Image.open(src_atlas_path)

# Делим атлас на тайлы
tiles = split_image(input_image, src_tile_width, src_tile_height)

# Расширяем каждый тайл
for index_y in range(tiles.shape[0]):
    for index_x in range(tiles.shape[1]):
        tiles[index_y, index_x] = expand_image(tiles[index_y, index_x], border_size)

# Объединяем тайлы в новый атлас
expanded_atlas, xml_doc = join_tiles(tiles, border_size)
expanded_atlas.save(result_atlas_path)
write_xml(xml_doc, result_xml_path)
