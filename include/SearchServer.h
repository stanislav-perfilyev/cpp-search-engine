#pragma once

#include <string>
#include <vector>

#include "InvertedIndex.h"
#include "exceptions.h"

struct RelativeIndex {
    size_t doc_id;
    float  rank;

    bool operator==(const RelativeIndex& other) const {
        return (doc_id == other.doc_id && rank == other.rank);
    }
};

/**
 * @brief Processes full-text search queries against an InvertedIndex.
 *
 * Results are ranked by relevance (highest rank first) and capped at
 * max_responses per query.  The return value of search() must not be
 * discarded — it is annotated [[nodiscard]].
 * Throws QueryError for malformed queries.
 */
class SearchServer {
public:
    /**
     * @param idx          Reference to an already-populated InvertedIndex.
     * @param max_responses Maximum results per query (default 5).
     */
    explicit SearchServer(InvertedIndex& idx, int max_responses = 5)
        : _index(idx), _max_responses(max_responses) {}

    /**
     * @brief Run a batch of search queries.
     * @param queries_input Query strings (from requests.json).
     * @return Per-query ranked list of (doc_id, rank) results.
     * @throws QueryError if a query is empty.
     */
    [[nodiscard]] std::vector<std::vector<RelativeIndex>>
        search(const std::vector<std::string>& queries_input);

private:
    InvertedIndex& _index;
    int            _max_responses;
};
