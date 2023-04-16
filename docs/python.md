## NumPy

В NumPy измерения (dimensions) называются осями (axes) ([пруф](https://numpy.org/doc/stable/user/quickstart.html#the-basics)).

## scikit-image

В scikit-image изображения - это просто массивы NumPy ([пруф](https://scikit-image.org/docs/stable/user_guide/data_types.html)).
Это позволяет, например, использовать функцию
[pad](https://numpy.org/doc/stable/reference/generated/numpy.pad.html) для расширения изображения-массива.
Grayscale изображения хранятся в двумерном массиве (высота, ширина), а цветные - в трёхмерном (высота, ширина, каналы)
([пруф](https://scikit-image.org/docs/stable/user_guide/numpy_images.html#coordinate-conventions)).
Порядок каналов: RGBA ([пруф](https://scikit-image.org/docs/stable/user_guide/data_types.html#working-with-opencv)).
