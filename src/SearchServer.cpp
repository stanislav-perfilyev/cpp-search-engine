#include "SearchServer.h"
#include <sstream>
#include <set>
#include <algorithm>
#include <map>

// ── private helpers ───────────────────────────────────────────────────────────

std::vector<std::pair<std::string, size_t>>
SearchServer::buildSortedWordList(const std::string& query) {
    std::istringstream iss(query);
    std::set<std::string> unique_words;
    std::string word;
    while (iss >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        unique_words.insert(word);
    }
    std::vector<std::pair<std::string, size_t>> words_with_freq;
    for (const auto& w : unique_words) {
        auto entries = _index.GetWordCount(w);
        size_t total = 0;
        for (const auto& e : entries) total += e.count;
        words_with_freq.push_back({w, total});
    }
    std::sort(words_with_freq.begin(), words_with_freq.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    return words_with_freq;
}

std::map<size_t, float>
SearchServer::computeDocRelevance(
    const std::vector<std::pair<std::string, size_t>>& words) {
    std::map<size_t, float> doc_relevance;
    for (const auto& e : _index.GetWordCount(words.front().first)) {
        doc_relevance[e.doc_id] = static_cast<float>(e.count);
    }
    for (size_t i = 1; i < words.size() && !doc_relevance.empty(); ++i) {
        std::map<size_t, size_t> by_doc;
        for (const auto& e : _index.GetWordCount(words[i].first)) {
            by_doc[e.doc_id] = e.count;
        }
        for (auto it = doc_relevance.begin(); it != doc_relevance.end();) {
            auto found = by_doc.find(it->first);
            if (found == by_doc.end()) {
                it = doc_relevance.erase(it);
            } else {
                it->second += static_cast<float>(found->second);
                ++it;
            }
        }
    }
    return doc_relevance;
}

std::vector<RelativeIndex>
SearchServer::searchSingle(const std::string& query) {
    auto words = buildSortedWordList(query);
    if (words.empty()) return {};
    auto doc_relevance = computeDocRelevance(words);
    if (doc_relevance.empty()) return {};

    float max_rel = 0;
    for (const auto& [doc_id, rel] : doc_relevance) {
        if (rel > max_rel) max_rel = rel;
    }
    std::vector<RelativeIndex> result;
    result.reserve(doc_relevance.size());
    for (const auto& [doc_id, rel] : doc_relevance) {
        result.push_back({doc_id, rel / max_rel});
    }
    std::sort(result.begin(), result.end(),
              [](const RelativeIndex& a, const RelativeIndex& b) {
                  return a.rank != b.rank ? a.rank > b.rank : a.doc_id < b.doc_id;
              });
    if (result.size() > static_cast<size_t>(_max_responses)) {
        result.resize(static_cast<size_t>(_max_responses));
    }
    return result;
}

// ── public API ────────────────────────────────────────────────────────────────

std::vector<std::vector<RelativeIndex>>
SearchServer::search(const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> result;
    result.reserve(queries_input.size());
    for (const auto& query : queries_input) {
        result.push_back(searchSingle(query));
    }
    return result;
}
