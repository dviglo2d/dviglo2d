От апстрима отличается только тем, что удалены все подмодули:

```
git submodule deinit external/SDL
rmdir /s /q .git\modules\external\SDL
git rm external/SDL

git submodule deinit external/flac
rmdir /s /q .git\modules\external\flac
git rm external/flac

git submodule deinit external/libgme
rmdir /s /q .git\modules\external\libgme
git rm external/libgme

git submodule deinit external/libxmp
:: Тут путь отличается!
rmdir /s /q .git\modules\libxmp
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
