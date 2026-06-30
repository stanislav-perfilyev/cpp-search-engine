#pragma once

#include <stdexcept>
#include <string>

/**
 * @brief Base exception for all search-engine errors.
 *
 * Hierarchy:
 *   SearchEngineError
 *   ├── ConfigError      — config.json missing / malformed / version mismatch
 *   ├── IndexError       — failure during document indexing
 *   └── QueryError       — invalid search query
 */
class SearchEngineError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/// config.json is missing, unreadable, malformed or has wrong version.
class ConfigError : public SearchEngineError {
    using SearchEngineError::SearchEngineError;
};

/// Failure while building or updating the inverted index.
class IndexError : public SearchEngineError {
    using SearchEngineError::SearchEngineError;
};

/// Invalid or empty search query.
class QueryError : public SearchEngineError {
    using SearchEngineError::SearchEngineError;
};
