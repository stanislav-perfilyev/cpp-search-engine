# Search Engine (локальный поисковик)

Проект индексирует текстовые документы и выполняет поиск по запросам из JSON-файла.

## Что делает приложение

- загружает список документов из `config.json`;
- строит инвертированный индекс (`word -> [{doc_id, count}, ...]`);
- обрабатывает запросы из `requests.json`;
- возвращает топ результатов по релевантности;
- сохраняет выдачу в `answers.json`.

## Структура проекта

```text
search_engine/
  include/
    ConverterJSON.h
    InvertedIndex.h
    SearchServer.h
  src/
    ConverterJSON.cpp
    InvertedIndex.cpp
    SearchServer.cpp
    search_app.cpp
  tests/
    main.cpp
  resources/
    file001.txt
    file002.txt
    file003.txt
    file004.txt
  scripts/
    run_search.bat
  CMakeLists.txt
  config.json
  requests.json
  answers.json
```

## Требования

- C++20;
- CMake 3.15+;
- компилятор GCC/Clang/MSVC;
- доступ в интернет при первой конфигурации CMake (подтягиваются `nlohmann/json` и `googletest`).

## Сборка

Ниже команды для PowerShell на Windows.

```powershell
Set-Location "C:\Users\stasp\OneDrive\c+study\search_engine"
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug
```

Если `cmake` не в `PATH`, собирайте из CLion или через встроенный CMake IDE.

## Запуск

Запуск приложения:

```powershell
Set-Location "C:\Users\stasp\OneDrive\c+study\search_engine"
.\cmake-build-debug\search_app.exe
```

Запуск всех тестов:

```powershell
Set-Location "C:\Users\stasp\OneDrive\c+study\search_engine"
.\cmake-build-debug\search_engine_tests.exe
```

Запуск одного теста:

```powershell
Set-Location "C:\Users\stasp\OneDrive\c+study\search_engine"
.\cmake-build-debug\search_engine_tests.exe --gtest_filter=TestCaseSearchServer.TestTop5
```

## Формат `config.json`

Пример:

```json
{
  "config": {
    "name": "SkillboxSearchEngine",
    "version": "0.1",
    "max_responses": 5
  },
  "files": [
    "resources/file001.txt",
    "resources/file002.txt",
    "resources/file003.txt",
    "resources/file004.txt"
  ]
}
```

Поля:

- `name` - имя приложения (печатается при старте);
- `version` - должна быть `0.1`;
- `max_responses` - максимум результатов на один запрос;
- `files` - список документов для индексации.

## Важное про пути

`ConverterJSON` ищет `config.json` в двух местах:

1. текущая директория;
2. `../config.json`.

Далее пути считаются относительно директории найденного `config.json`:

- файлы из `config.files`;
- `requests.json`;
- выходной `answers.json`.

Это позволяет запускать бинарник как из корня проекта, так и из `cmake-build-debug`.

## Формат `requests.json`

```json
{
  "requests": [
    "milk water",
    "tea coffee"
  ]
}
```

Если файл отсутствует, приложение пишет warning и возвращает пустой список запросов.

## Алгоритм поиска (`SearchServer`)

Для каждого запроса:

1. запрос разбивается на уникальные слова;
2. слова сортируются по общей частоте (от редких к частым);
3. выполняется пересечение документов (в выдаче остаются только документы, где есть все слова);
4. абсолютная релевантность = сумма `count` по словам;
5. относительная релевантность = `abs / max_abs`;
6. сортировка: сначала `rank` по убыванию, при равенстве `doc_id` по возрастанию;
7. обрезка до `max_responses`.

## Формат `answers.json`

`ConverterJSON::putAnswers` формирует:

- `result: false`, если документов нет;
- `result: true` + `docid` + `rank`, если найден ровно один документ;
- `result: true` + массив `relevance`, если найдено несколько.

Пример:

```json
{
  "answers": {
    "request001": {
      "result": true,
      "docid": 3,
      "rank": 0.75
    },
    "request002": {
      "result": true,
      "relevance": [
        { "docid": 1, "rank": 1.0 },
        { "docid": 3, "rank": 0.5 }
      ]
    },
    "request003": {
      "result": false
    }
  }
}
```

## Логирование и предупреждения

Типичные сообщения:

- `Warning: cannot open file ...` - один из файлов из `config.files` не открылся;
- `Warning: requests.json file is missing: ...` - файл запросов не найден.

Если файл документа отсутствует, его `doc_id` не «съезжает»: в индексацию попадает пустая строка на той же позиции.

## Тесты

В проекте используются Google Test (`gtest_main` подключен в `CMakeLists.txt`).

Покрыты группы сценариев:

- `InvertedIndex`: базовый подсчет частот и отсутствие слова;
- `SearchServer`: базовый поиск и контрольный `Top5`;
- `ConverterJSON`: работа с путями, формат `answers.json`, ошибки/границы `config.json` и `requests.json`.

Запуск через CTest (из build-директории):

```powershell
Set-Location "C:\Users\stasp\OneDrive\c+study\search_engine\cmake-build-debug"
ctest --output-on-failure
```

## Коротко о реализации

- `InvertedIndex` строится многопоточно (по потоку на документ);
- все слова приводятся к нижнему регистру;
- хранилище частот: `std::map<std::string, std::vector<Entry>>`.
