Подмодули качаются без истории (включён shallow).

Добавление подмодуля на примере SDL:

```
git submodule add https://github.com/dviglo2d/sdl libs/third_party/sdl/repo
git config .gitmodules submodule.libs/third_party/sdl/repo.shallow true
```
