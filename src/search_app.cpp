#include <iostream>
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"

int main() {
    try {

        // Создаем экземпляр ConverterJSON
        ConverterJSON converter;

        // Читаем текстовые документы
        std::cout << "Loading documents..." << std::endl;
        auto docs = converter.GetTextDocuments();
        std::cout << "Loaded " << docs.size() << " documents" << std::endl;

        // Создаем инвертированный индекс
        std::cout << "Building inverted index..." << std::endl;
        InvertedIndex index;
        index.UpdateDocumentBase(docs);
        std::cout << "Index built successfully" << std::endl;

        // Читаем поисковые запросы
        std::cout << "Reading search requests..." << std::endl;
        auto requests = converter.GetRequests();
        std::cout << "Found " << requests.size() << " requests" << std::endl;

        // Получаем максимальное количество ответов
        int max_responses = converter.GetResponsesLimit();
        std::cout << "Max responses per query: " << max_responses << std::endl;

        // Создаем поисковый сервер и выполняем поиск
        std::cout << "Performing search..." << std::endl;
        SearchServer server(index, max_responses);
        auto search_results = server.search(requests);

        // Преобразуем результаты в формат для сохранения
        std::vector<std::vector<std::pair<int, float>>> answers;
        for (const auto& query_result : search_results) {
            std::vector<std::pair<int, float>> query_answer;
            for (const auto& result : query_result) {
                query_answer.push_back({static_cast<int>(result.doc_id), result.rank});
            }
            answers.push_back(query_answer);
        }

        // Сохраняем результаты
        std::cout << "Saving results..." << std::endl;
        converter.putAnswers(answers);
        std::cout << "Results saved to answers.json" << std::endl;

        // Выводим статистику
        std::cout << "\n=== Search Statistics ===" << std::endl;
        for (size_t i = 0; i < requests.size(); ++i) {
            std::cout << "Query " << (i + 1) << ": \"" << requests[i] << "\"" << std::endl;
            std::cout << "  Found " << answers[i].size() << " documents" << std::endl;
            if (!answers[i].empty()) {
                std::cout << "  Top result: doc_id=" << answers[i][0].first
                          << ", rank=" << answers[i][0].second << std::endl;
            }
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

