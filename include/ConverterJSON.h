#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

/**
 * Класс для работы с JSON-файлами
 */
class ConverterJSON {
public:
    ConverterJSON() = default;

    /**
     * Метод получения содержимого файлов
     * @return Возвращает список с содержимым файлов перечисленных
     * в config.json
     */
    std::vector<std::string> GetTextDocuments();

    /**
     * Метод считывает поле max_responses для определения предельного
     * количества ответов на один запрос
     * @return максимальное количество ответов на один запрос
     */
    int GetResponsesLimit();

    /**
     * Метод получения запросов из файла requests.json
     * @return возвращает список запросов из файла requests.json
     */
    std::vector<std::string> GetRequests();

    /**
     * Положить в файл answers.json результаты поисковых запросов
     */
    void putAnswers(std::vector<std::vector<std::pair<int, float>>> answers);

private:
    nlohmann::json LoadConfig();
    std::string ResolveConfigPath() const;
};

