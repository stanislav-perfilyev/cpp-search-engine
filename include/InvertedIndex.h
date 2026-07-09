#pragma once

#include <map>
#include <string>
#include <vector>

#include "exceptions.h"

/// @brief Per-document term frequency: number of times a word appears in doc_id.
struct Entry {
    size_t doc_id, count;

    bool operator==(const Entry& other) const {
        return (doc_id == other.doc_id && count == other.count);
    }
};

/**
 * @brief Inverted index over a document collection.
 *
 * Call UpdateDocumentBase() to (re-)index documents, then query with
 * GetWordCount().  All query results must be used — [[nodiscard]].
 * Throws IndexError if indexing fails.
 */
class InvertedIndex {
public:
    InvertedIndex() = default;

    /**
     * @brief (Re-)build the index from the given document texts.
     * @param input_docs Document contents (one string per document).
     * @throws IndexError on internal failure.
     */
    void UpdateDocumentBase(std::vector<std::string> input_docs);

    /**
     * @brief Return per-document occurrence counts for a word.
     * @param word The word to look up.
     * @return Sorted list of (doc_id, count) entries; empty if word absent.
     */
    [[nodiscard]] std::vector<Entry> GetWordCount(const std::string& word);

private:
    std::vector<std::string>              docs;
    std::map<std::string, std::vector<Entry>> freq_dictionary;
};
