#!/usr/bin/env python3

# Удаляет пробелы в конце строк

import os


# Определяет, какой разделитель используется в файле
def get_newline(file_path: str) -> str:
    with open(file_path, 'rb') as file:
        content = file.read()

    if b'\r\n' in content:
        return '\r\n'
    else:
        return '\n'


# Удаляет пробелы в конце строк
def remove_trailing_spaces(file_path: str):
    with open(file_path, 'r', encoding='utf-8') as file:
        lines: list[str] = file.readlines()

    # Сохраняем формат разделителя строк
    nel: str = get_newline(file_path)

    with open(file_path, 'w', encoding='utf-8', newline=nel) as file:
        for line in lines:
            file.write(line.rstrip() + '\n')


if __name__ == '__main__':
    repo_dir: str = os.path.dirname(os.path.realpath(__file__))
    exts: set[str] = {'.bat', '.sh', '.md', '.hpp', '.cpp', '.txt', '.py', '.yml', '.editorconfig', '.gitattributes', '.gitignore',
                      '.frag', '.vert', '.xml'}

    for dir, subdirs, files in os.walk(repo_dir):
        # Игнорируемые папки
        if dir == repo_dir:
            subdirs.remove('.git')
            subdirs.remove('third_party')
        elif dir == os.path.join(repo_dir, 'engine'):
            subdirs.remove('third_party')

        for file in files:
            if any(file.endswith(ext) for ext in exts):
                file_path = os.path.join(dir, file)
                remove_trailing_spaces(file_path)
                print('Обработан файл: ' + file_path)
