#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#include "exceptions.h"

/**
 * @brief Works with the search-engine JSON configuration and result files.
 *
 * Reads config.json / requests.json; writes answers.json.
 * Throws ConfigError on any config problem (missing file, bad JSON, version
 * mismatch).  Callers must not ignore the return values of Get* methods —
 * they are annotated [[nodiscard]].
 */
class ConverterJSON {
public:
    ConverterJSON() = default;

    /**
     * @brief Read document contents listed in config.json.
     * @return One string per configured file (empty string for missing files).
     * @throws ConfigError if config.json is absent, malformed, or version mismatch.
     */
    [[nodiscard]] std::vector<std::string> GetTextDocuments();

    /**
     * @brief Read max_responses from config.json.
     * @return Maximum number of search results per query (default 5).
     * @throws ConfigError if config.json is absent or malformed.
     */
    [[nodiscard]] int GetResponsesLimit();

    /**
     * @brief Read search queries from requests.json.
     * @return List of query strings; empty if requests.json is absent.
     */
    [[nodiscard]] std::vector<std::string> GetRequests();

    /**
     * @brief Write search results to answers.json.
     * @param answers Per-query list of (doc_id, rank) pairs.
     */
    void putAnswers(std::vector<std::vector<std::pair<int, float>>> answers);

private:
    [[nodiscard]] nlohmann::json LoadConfig();
    [[nodiscard]] std::string    ResolveConfigPath() const;
};
