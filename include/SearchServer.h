#pragma once

#include "InvertedIndex.h"
#include <vector>
#include <string>

struct RelativeIndex {
    size_t doc_id;
    float rank;

    bool operator==(const RelativeIndex& other) const {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};

class SearchServer {
public:
    /**
     * @param idx в конструктор класса передаётся ссылка на класс InvertedIndex,
     * чтобы SearchServer мог узнать частоту слов встречаемых в запросе
     * @param max_responses максимальное количество результатов на запрос (по умолчанию 5)
     */
    SearchServer(InvertedIndex& idx, int max_responses = 5)
        : _index(idx), _max_responses(max_responses) {}

    /**
     * Метод обработки поисковых запросов
     * @param queries_input поисковые запросы взятые из файла requests.json
     * @return возвращает отсортированный список релевантных ответов для заданных запросов
     */
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input);

private:
    InvertedIndex& _index;
    int _max_responses;
};

