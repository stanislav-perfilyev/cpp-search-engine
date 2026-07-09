#include <iostream>
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"

static void printStatistics(
    const std::vector<std::string>& requests,
    const std::vector<std::vector<std::pair<int, float>>>& answers) {
    std::cout << "\n=== Search Statistics ===\n";
    for (size_t i = 0; i < requests.size(); ++i) {
        std::cout << "Query " << (i + 1) << ": \"" << requests[i] << "\"\n";
        std::cout << "  Found " << answers[i].size() << " documents\n";
        if (!answers[i].empty()) {
            std::cout << "  Top result: doc_id=" << answers[i][0].first
                      << ", rank=" << answers[i][0].second << '\n';
        }
    }
}

static void runSearch() {
    ConverterJSON converter;

    std::cout << "Loading documents...\n";
    auto docs = converter.GetTextDocuments();
    std::cout << "Loaded " << docs.size() << " documents\n";

    std::cout << "Building inverted index...\n";
    InvertedIndex index;
    index.UpdateDocumentBase(docs);
    std::cout << "Index built successfully\n";

    std::cout << "Reading search requests...\n";
    auto requests = converter.GetRequests();
    std::cout << "Found " << requests.size() << " requests\n";

    int max_responses = converter.GetResponsesLimit();
    std::cout << "Max responses per query: " << max_responses << '\n';

    std::cout << "Performing search...\n";
    SearchServer server(index, max_responses);
    auto search_results = server.search(requests);

    std::vector<std::vector<std::pair<int, float>>> answers;
    for (const auto& query_result : search_results) {
        std::vector<std::pair<int, float>> query_answer;
        for (const auto& r : query_result) {
            query_answer.push_back({static_cast<int>(r.doc_id), r.rank});
        }
        answers.push_back(query_answer);
    }

    std::cout << "Saving results...\n";
    converter.putAnswers(answers);
    std::cout << "Results saved to answers.json\n";

    printStatistics(requests, answers);
}

int main() {
    try {
        runSearch();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
