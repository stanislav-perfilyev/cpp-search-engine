#include "SearchServer.h"
#include <sstream>
#include <set>
#include <algorithm>
#include <map>

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> result;

    for (const auto& query : queries_input) {
        std::vector<RelativeIndex> query_result;

        // Разбиваем запрос на слова
        std::istringstream iss(query);
        std::set<std::string> unique_words;
        std::string word;

        while (iss >> word) {
            // Преобразуем слово в нижний регистр
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            unique_words.insert(word);
        }

        // Создаем список слов с их частотой встречаемости
        std::vector<std::pair<std::string, size_t>> words_with_freq;
        for (const auto& w : unique_words) {
            auto entries = _index.GetWordCount(w);
            size_t total_count = 0;
            for (const auto& entry : entries) {
                total_count += entry.count;
            }
            words_with_freq.push_back({w, total_count});
        }

        // Сортируем слова по возрастанию частоты
        std::sort(words_with_freq.begin(), words_with_freq.end(),
                  [](const auto& a, const auto& b) {
                      return a.second < b.second;
                  });

        if (words_with_freq.empty()) {
            result.push_back(query_result);
            continue;
        }

        // Пересекаем документы по словам, начиная с самого редкого
        std::map<size_t, float> doc_relevance;
        auto first_entries = _index.GetWordCount(words_with_freq.front().first);
        for (const auto& entry : first_entries) {
            doc_relevance[entry.doc_id] = static_cast<float>(entry.count);
        }

        for (size_t i = 1; i < words_with_freq.size() && !doc_relevance.empty(); ++i) {
            auto entries = _index.GetWordCount(words_with_freq[i].first);
            std::map<size_t, size_t> entries_by_doc;
            for (const auto& entry : entries) {
                entries_by_doc[entry.doc_id] = entry.count;
            }

            for (auto it = doc_relevance.begin(); it != doc_relevance.end();) {
                auto found = entries_by_doc.find(it->first);
                if (found == entries_by_doc.end()) {
                    it = doc_relevance.erase(it);
                } else {
                    it->second += static_cast<float>(found->second);
                    ++it;
                }
            }
        }

        if (doc_relevance.empty()) {
            result.push_back(query_result);
            continue;
        }

        // Находим максимальную абсолютную релевантность
        float max_relevance = 0;
        for (const auto& [doc_id, relevance] : doc_relevance) {
            if (relevance > max_relevance) {
                max_relevance = relevance;
            }
        }

        // Вычисляем относительную релевантность и формируем результат
        for (const auto& [doc_id, abs_relevance] : doc_relevance) {
            float relative_relevance = abs_relevance / max_relevance;
            query_result.push_back({doc_id, relative_relevance});
        }

        // Сортируем результаты по убыванию релевантности
        std::sort(query_result.begin(), query_result.end(),
                  [](const RelativeIndex& a, const RelativeIndex& b) {
                      if (a.rank != b.rank) {
                          return a.rank > b.rank;
                      }
                      return a.doc_id < b.doc_id;
                  });

        // Ограничиваем количество результатов
        if (query_result.size() > static_cast<size_t>(_max_responses)) {
            query_result.resize(_max_responses);
        }

        result.push_back(query_result);
    }

    return result;
}

