Литература:
1. https://www.reddit.com/r/opengl/comments/1eboyoc/vsync_and_input_lag/
2. https://danluu.com/latency-mitigation/
3. https://github.com/yshui/picom/issues/592
4. https://developer.android.com/games/sdk/frame-pacing

Если включена вертикалка, то отрендеренный кадр "устаревает", пока ждёт показа.
Большая пауза между считыванием ввода (апдейтом) и показом - это и есть инпут лаг.
Если отключить вертикалку и включить ограничение ФПС, то пауза будет перед апдейтом,
то есть при том же ФПС инпут лаг будет меньше.
