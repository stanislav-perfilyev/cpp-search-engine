#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include "gtest/gtest.h"
#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"
#include "exceptions.h"
#include <nlohmann/json.hpp>

using namespace std;

namespace {
class CurrentPathGuard {
public:
    CurrentPathGuard() : original_path(std::filesystem::current_path()) {}
    ~CurrentPathGuard() {
        std::error_code ec;
        std::filesystem::current_path(original_path, ec);
    }

private:
    std::filesystem::path original_path;
};

std::filesystem::path MakeUniqueTestRoot(const std::string& prefix) {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = std::filesystem::temp_directory_path() / (prefix + "_" + suffix);
    std::filesystem::create_directories(root);
    return root;
}

void WriteTextFile(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    out << content;
}

void WriteDefaultConfig(const std::filesystem::path& root,
                        const std::string& version = "0.1",
                        const std::string& files_json = "\"resources/file001.txt\"") {
    std::ostringstream config;
    config << "{\n"
           << "  \"config\": {\n"
           << "    \"name\": \"SkillboxSearchEngine\",\n"
           << "    \"version\": \"" << version << "\",\n"
           << "    \"max_responses\": 5\n"
           << "  },\n"
           << "  \"files\": [" << files_json << "]\n"
           << "}\n";
    WriteTextFile(root / "config.json", config.str());
}

void CleanupTestRoot(const std::filesystem::path& root) {
    std::error_code set_cwd_ec;
    std::filesystem::current_path(std::filesystem::temp_directory_path(), set_cwd_ec);

    std::error_code cleanup_ec;
    std::filesystem::remove_all(root, cleanup_ec);
    if (cleanup_ec) {
        std::cerr << "Warning: failed to remove temp dir " << root.string()
                  << ": " << cleanup_ec.message() << '\n';
    }
}
}

// Тесты для InvertedIndex
void TestInvertedIndexFunctionality(
    const vector<string>& docs,
    const vector<string>& requests,
    const std::vector<vector<Entry>>& expected
) {
    std::vector<std::vector<Entry>> result;
    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);

    for (auto& request : requests) {
        std::vector<Entry> word_count = idx.GetWordCount(request);
        result.push_back(word_count);
    }

    ASSERT_EQ(result, expected);
}

TEST(TestCaseInvertedIndex, TestBasic) {
    const vector<string> docs = {
        "london is the capital of great britain",
        "big ben is the nickname for the great bell of the striking clock"
    };
    const vector<string> requests = {"london", "the"};
    const vector<vector<Entry>> expected = {
        {
            {0, 1}
        },
        {
            {0, 1}, {1, 3}
        }
    };

    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST(TestCaseInvertedIndex, TestBasic2) {
    const vector<string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"
    };
    const vector<string> requests = {"milk", "water", "cappuccino"};
    const vector<vector<Entry>> expected = {
        {
            {0, 4}, {1, 1}, {2, 5}
        },
        {
            {0, 3}, {1, 2}, {2, 5}
        },
        {
            {3, 1}
        }
    };

    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord) {
    const vector<string> docs = {
        "a b c d e f g h i j k l",
        "statement"
    };
    const vector<string> requests = {"m", "statement"};
    const vector<vector<Entry>> expected = {
        {},
        {
            {1, 1}
        }
    };

    TestInvertedIndexFunctionality(docs, requests, expected);
}

// Тесты для SearchServer
TEST(TestCaseSearchServer, TestSimple) {
    const vector<string> docs = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino"
    };
    const vector<string> request = {"milk water", "sugar"};
    const std::vector<vector<RelativeIndex>> expected = {
        {
            {2, 1},
            {0, 0.7f},
            {1, 0.3f}
        },
        {}
    };

    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result, expected);
}

TEST(TestCaseSearchServer, TestTop5) {
    const vector<string> docs = {
        "milk water",
        "milk milk water",
        "milk water water",
        "milk milk water water",
        "milk milk milk water water",
        "milk water water water",
        "milk milk milk milk water",
        "tea coffee"
    };
    const vector<string> request = {"milk water"};
    const std::vector<vector<RelativeIndex>> expected = {
        {
            {4, 1.0f},
            {6, 1.0f},
            {3, 0.8f},
            {5, 0.8f},
            {1, 0.6f}
        }
    };

    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);
    SearchServer srv(idx);
    std::vector<vector<RelativeIndex>> result = srv.search(request);
    ASSERT_EQ(result, expected);
}

TEST(TestCaseConverterJSON, TestDocIdDoesNotShiftWhenFileMissing) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_docid_test");
    WriteDefaultConfig(test_root, "0.1",
        "\"resources/file001.txt\",\"resources/file002.txt\",\"resources/file003.txt\"");
    WriteTextFile(test_root / "resources" / "file001.txt", "alpha beta");
    WriteTextFile(test_root / "resources" / "file003.txt", "alpha");
    fs::current_path(test_root);

    ConverterJSON converter;
    auto docs = converter.GetTextDocuments();
    ASSERT_EQ(docs.size(), 3);
    ASSERT_TRUE(docs[1].empty());

    InvertedIndex idx;
    idx.UpdateDocumentBase(docs);
    SearchServer srv(idx);
    auto result = srv.search({"alpha"});

    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result[0].size(), 2);
    EXPECT_EQ(result[0][0].doc_id, 0);
    EXPECT_EQ(result[0][1].doc_id, 2);

    CleanupTestRoot(test_root);
}

TEST(TestCaseConverterJSON, TestSingleResultFormat) {
    ConverterJSON converter;
    converter.putAnswers({{{3, 0.75f}}});

    const std::filesystem::path answers_path = std::filesystem::exists("../config.json")
            ? std::filesystem::path("../answers.json")
            : std::filesystem::path("answers.json");

    std::ifstream input(answers_path);
    ASSERT_TRUE(input.is_open());

    nlohmann::json parsed;
    input >> parsed;

    ASSERT_TRUE(parsed.contains("answers"));
    ASSERT_TRUE(parsed["answers"].contains("request001"));

    const auto& request = parsed["answers"]["request001"];
    ASSERT_TRUE(request.contains("result"));
    ASSERT_TRUE(request["result"].get<bool>());
    ASSERT_TRUE(request.contains("docid"));
    ASSERT_EQ(request["docid"].get<int>(), 3);
    ASSERT_TRUE(request.contains("rank"));
    ASSERT_FLOAT_EQ(request["rank"].get<float>(), 0.75f);
    ASSERT_FALSE(request.contains("relevance"));
}

TEST(TestCaseConverterJSON, TestConfigFileMissingThrows) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_cfg_missing");
    const fs::path run_dir = test_root / "run";
    fs::create_directories(run_dir);
    fs::current_path(run_dir);

    ConverterJSON converter;
    EXPECT_THROW(static_cast<void>(converter.GetTextDocuments()), ConfigError);

    CleanupTestRoot(test_root);
}

TEST(TestCaseConverterJSON, TestConfigParseErrorThrows) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_cfg_parse");
    WriteTextFile(test_root / "config.json", "{ bad json }");
    fs::current_path(test_root);

    ConverterJSON converter;
    EXPECT_THROW(static_cast<void>(converter.GetTextDocuments()), ConfigError);

    CleanupTestRoot(test_root);
}

TEST(TestCaseConverterJSON, TestConfigVersionMismatchThrows) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_cfg_version");
    WriteDefaultConfig(test_root, "0.2");
    fs::current_path(test_root);

    ConverterJSON converter;
    EXPECT_THROW(static_cast<void>(converter.GetTextDocuments()), ConfigError);

    CleanupTestRoot(test_root);
}

TEST(TestCaseConverterJSON, TestAllConfiguredFilesMissingKeepDocIndexes) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_all_missing_docs");
    WriteDefaultConfig(
        test_root,
        "0.1",
        "\"resources/file001.txt\",\"resources/file002.txt\",\"resources/file003.txt\"");
    fs::current_path(test_root);

    ConverterJSON converter;
    const auto docs = converter.GetTextDocuments();

    ASSERT_EQ(docs.size(), 3);
    EXPECT_TRUE(docs[0].empty());
    EXPECT_TRUE(docs[1].empty());
    EXPECT_TRUE(docs[2].empty());

    CleanupTestRoot(test_root);
}

TEST(TestCaseConverterJSON, TestRequestsJsonMissingReturnsEmpty) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_requests_missing");
    WriteDefaultConfig(test_root);
    fs::current_path(test_root);

    ConverterJSON converter;
    const auto requests = converter.GetRequests();
    EXPECT_TRUE(requests.empty());

    CleanupTestRoot(test_root);
}

TEST(TestCaseConverterJSON, TestEmptyResultFormat) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_answers_empty");
    WriteDefaultConfig(test_root);
    fs::current_path(test_root);

    ConverterJSON converter;
    converter.putAnswers({{}});

    {
        std::ifstream input(test_root / "answers.json");
        ASSERT_TRUE(input.is_open());

        nlohmann::json parsed;
        input >> parsed;

        const auto& request = parsed["answers"]["request001"];
        ASSERT_TRUE(request.contains("result"));
        ASSERT_FALSE(request["result"].get<bool>());
        ASSERT_FALSE(request.contains("docid"));
        ASSERT_FALSE(request.contains("rank"));
        ASSERT_FALSE(request.contains("relevance"));
    }

    CleanupTestRoot(test_root);
}

TEST(TestCaseConverterJSON, TestMultipleResultsFormat) {
    namespace fs = std::filesystem;
    CurrentPathGuard guard;

    const fs::path test_root = MakeUniqueTestRoot("search_engine_answers_multi");
    WriteDefaultConfig(test_root);
    fs::current_path(test_root);

    ConverterJSON converter;
    converter.putAnswers({{{1, 1.0f}, {3, 0.5f}}});

    {
        std::ifstream input(test_root / "answers.json");
        ASSERT_TRUE(input.is_open());

        nlohmann::json parsed;
        input >> parsed;

        const auto& request = parsed["answers"]["request001"];
        ASSERT_TRUE(request.contains("result"));
        ASSERT_TRUE(request["result"].get<bool>());
        ASSERT_TRUE(request.contains("relevance"));
        ASSERT_EQ(request["relevance"].size(), 2);
        ASSERT_EQ(request["relevance"][0]["docid"].get<int>(), 1);
        ASSERT_FLOAT_EQ(request["relevance"][0]["rank"].get<float>(), 1.0f);
        ASSERT_EQ(request["relevance"][1]["docid"].get<int>(), 3);
        ASSERT_FLOAT_EQ(request["relevance"][1]["rank"].get<float>(), 0.5f);
        ASSERT_FALSE(request.contains("docid"));
        ASSERT_FALSE(request.contains("rank"));
    }

    CleanupTestRoot(test_root);
}

