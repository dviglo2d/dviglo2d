От апстрима отличается только тем, что удалены все подмодули.

Процесс обновления SDL_mixer:
1) Через сайт обновить из апстрима ветку main форка https://github.com/dviglo2d/sdl_mixer
2) Скачать репозиторий: `git clone --recurse-submodules https://github.com/dviglo2d/sdl_mixer repo`
3) Создать ветку dv_гггг_мм_дд. Если нужна вторая ветка в тот же день, то назвать её dv_гггг_мм_дд_2
4) Удалить подмодули батником:

```
cd repo

git submodule deinit external/flac
rmdir /s /q .git\modules\external\flac
git rm external/flac

git submodule deinit external/libgme
rmdir /s /q .git\modules\external\libgme
git rm external/libgme

git submodule deinit external/libxmp
rmdir /s /q .git\modules\external\libxmp
git rm external/libxmp

git submodule deinit external/mpg123
rmdir /s /q .git\modules\external\mpg123
git rm external/mpg123

git submodule deinit external/ogg
rmdir /s /q .git\modules\external\ogg
git rm external/ogg

git submodule deinit external/opus
rmdir /s /q .git\modules\external\opus
git rm external/opus

git submodule deinit external/opusfile
rmdir /s /q .git\modules\external\opusfile
git rm external/opusfile

git submodule deinit external/tremor
rmdir /s /q .git\modules\external\tremor
git rm external/tremor

git submodule deinit external/vorbis
rmdir /s /q .git\modules\external\vorbis
git rm external/vorbis

git submodule deinit external/wavpack
rmdir /s /q .git\modules\external\wavpack
git rm external/wavpack
```

5) Назвать коммит: `Dviglo: Удалил подмодули`
6) В репозитории движка обновить ветку в файлах add_submodules.sh и update_submodules.sh
7) Запустить update_submodules.sh
