#!/usr/bin/env python3

# Не знаю почему, но scikit-image в Windows сжимает png файлы лучше, чем GIMP.
# Загружаем файл и тут же сохраняем его назад.
# в Linux файлы получаются больше, чем в Windows

from typing import TypeAlias
import numpy as np
import os
import skimage as ski


# Псевдоним для изображения, которое на самом деле просто массив NumPy
Image: TypeAlias = np.ndarray[tuple[int, ...], np.dtype[np.uint8]]


def resave(img_path: str):
    img: Image = ski.io.imread(img_path)
    assert(img.ndim == 3) # Изображение цветное
    assert(img.size) # Изображение не пустое
    assert(img.dtype == np.uint8) # Массив содержит байты

    ski.io.imsave(img_path, img)


if __name__ == '__main__':
    # Папка, в которой находится скрипт. В конце строки \ или /
    script_dir: str = os.path.dirname(os.path.abspath(__file__)) + os.path.sep

    resave(script_dir + 'input_atlas.png')
