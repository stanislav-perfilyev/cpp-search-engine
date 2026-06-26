#include "InvertedIndex.h"
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>

std::mutex index_mutex;

void InvertedIndex::UpdateDocumentBase(std::vector<std::string> input_docs) {
    docs = input_docs;
    freq_dictionary.clear();

    // Функция для индексации одного документа
    auto indexDocument = [this](size_t doc_id) {
        std::map<std::string, size_t> local_word_count;
        std::istringstream iss(docs[doc_id]);
        std::string word;

        // Подсчет слов в документе
        while (iss >> word) {
            // Преобразуем слово в нижний регистр
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            local_word_count[word]++;
        }

        // Добавление в общий частотный словарь
        std::lock_guard<std::mutex> lock(index_mutex);
        for (const auto& [word, count] : local_word_count) {
            freq_dictionary[word].push_back({doc_id, count});
        }
    };

    // Запуск индексации в отдельных потоках
    std::vector<std::thread> threads;
    for (size_t i = 0; i < docs.size(); ++i) {
        threads.emplace_back(indexDocument, i);
    }

    // Ожидание завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }
}

std::vector<Entry> InvertedIndex::GetWordCount(const std::string& word) {
    if (freq_dictionary.find(word) != freq_dictionary.end()) {
        auto result = freq_dictionary[word];
        // Сортируем по doc_id
        std::sort(result.begin(), result.end(), [](const Entry& a, const Entry& b) {
            return a.doc_id < b.doc_id;
        });
        return result;
    }
    return {};
}

