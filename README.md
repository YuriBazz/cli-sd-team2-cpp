# Bash C++ (cli-sd-team2-cpp)

Простой интерпретатор командной строки, поддерживающий команды:
* 🐱 `cat [FILE]` – выводит содержимое файла на экран
* 😱 `echo` – выводит на экран свои аргументы
* 🚽 `wc [FILE]` – выводит количество строк, слов и байт в файле
* 🎁 `pwd` – выводит текущую директорию
* 🚪 `exit [CODE]` – выходит из интерпретатора с указанным кодом (по умолчанию 0)
* 🔍 `grep` – программа для поиска шаблонов в файлах
  * **Usage:** `grep [option]... [pattern] [file]`
  * **Флаги:**
      - `-i` – выполнять поиск без учёта регистра
      - `-c` – выводить только количество выбранных строк
      - `-l` – выводить только имена файлов, содержащих выбранные строки
      - `-w` – искать выражение как отдельное слово
      - `-A num` – вывести `num` строк последующего контекста после каждого совпадения

## Архитектура

### Фазы выполнения

1. **Лексический анализ (lexer.l)** — разбивает входную строку на токены с помощью Flex
   - Поддерживает: слова, строки в кавычках ('/"'), переменные ($VAR), операторы (|, ;, =)
   - Обрабатывает экранирование (\n, \t, \\, \" внутри двойных кавычек)

2. **Синтаксический анализ (parser.y)** — строит AST согласно грамматике с помощью Bison

   program -> statement NEWLINE | program statement NEWLINE
   statement -> pipeline | assignment
   pipeline -> command | pipeline PIPE command
   command -> TOKEN_CAT argument_list | WORD argument_list
   assignment -> WORD_ASSIGN argument_list | WORD ASSIGN argument_list
   argument -> WORD | STRING | VARIABLE | NUMBER

3. **Выполнение (ast.cpp, commands.cpp)** — обход AST с подстановкой переменных
   - Environment::expand() заменяет $VAR на значения переменных
   - Pipeline::execute() создаёт процессы и pipe() между командами
   - Builtin-команды (cat, echo, wc, pwd, exit, grep) выполняются без fork() (кроме пайпов)
   - Внешние команды запускаются через execvp() с поиском в PATH

### Семантика подстановки переменных

- Переменные распознаются на этапе лексического анализа как токен VARIABLE
- Подстановка происходит в момент выполнения через Environment::expand()
- В двойных кавычках переменные раскрываются: echo "Hello $NAME" -> Hello John
- В одинарных кавычках подстановка НЕ выполняется: echo 'Hello $NAME' -> Hello $NAME
- Конкатенация: foo$bar распознаётся как единый аргумент с подстановкой
- Word splitting: x="a b"; echo $x -> два аргумента a и b

### Управление окружением

- Environment хранит переменные в unordered_map<string, string>
- При старте импортирует переменные окружения через environ
- Assignment::execute() создаёт/изменяет переменные
- Для внешних команд окружение пробрасывается через setenv()
- Поддерживается prefix-assignment: VAR=x cmd (временно установить переменную для команды)

### Обработка кодов возврата

- Pipeline::execute() сохраняет код возврата последней команды
- exit принимает аргумент: exit 1 завершает с кодом 1
- Переменная $? возвращает код последней выполненной команды

### Расширяемость

- Реестр команд (CommandRegistry) позволяет добавлять команды без изменения грамматики:
  registry.register("mycmd", [](args) { return new MyCommand(args); });
- Новые команды наследуются от Command и реализуют execute(env, inputFd, outputFd)

## Возможности

* Конвейеры (оператор |) с сохранением кода возврата
* Переменные сессии с подстановкой ($VAR)
* Присваивание переменных (VAR=value)
* Подстановка переменных в двойных кавычках
* Конкатенация слов и переменных (foo$bar)
* Word splitting для переменных с пробелами
* Переменные окружения
* Экранирование (', ", \n, \t)
* Prefix-assignment для команд (VAR=x cmd)
* Код возврата команд и exit N
* Внешние процессы через PATH

## Начало работы

### Предварительные требования

* C++17
* CMake 3.16+
* Flex (лексер)
* Bison >= 3.0 (на macOS: brew install bison)

### Запуск программы

#### Компиляция и запуск через CMake

```bash
git clone https://github.com/username/project.git
cd project
mkdir build && cd build
cmake .. -DBISON_EXECUTABLE=$(brew --prefix bison)/bin/bison
make -j$(nproc)
./bin/Bash
```

# Качество кода

### Автоматические проверки

#### clang-format (стиль кода)
- Конфигурация основана на Google Style
- Отступы: 4 пробела, длина строки: 100 символов

```bash
make format
```
#### cppcheck (статический анализ)

```bash
make analyze
```

## Тестирование

Проект использует Google Test для модульного тестирования.

### Запуск тестов локально

```bash
mkdir build && cd build
cmake ..
make
ctest --output-on-failure
./tests/bash_tests --gtest_filter=ModuleName.*
```

## Непрерывная интеграция (CI)

GitHub Actions запускает при push в ветки и pull request в main:
- Установка зависимостей (flex, bison, cppcheck, clang-format)
- Проверка форматирования clang-format
- Сборка с флагами -Wall -Wextra -Werror
- Статический анализ cppcheck
- Запуск всех юнит-тестов через ctest
