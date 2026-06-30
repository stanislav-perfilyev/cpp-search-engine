#include "ConverterJSON.h"
#include "exceptions.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

using json = nlohmann::json;

namespace {
constexpr const char* kExpectedVersion = "0.1";
}

std::string ConverterJSON::ResolveConfigPath() const {
    namespace fs = std::filesystem;

    if (fs::exists("config.json")) {
        return fs::path("config.json").lexically_normal().string();
    }
    if (fs::exists("../config.json")) {
        return fs::path("../config.json").lexically_normal().string();
    }
    throw ConfigError("config file is missing");
}

json ConverterJSON::LoadConfig() {
    const std::string config_path = ResolveConfigPath();
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw ConfigError("config file is missing");
    }

    json config;
    try {
        config_file >> config;
    } catch (const json::parse_error&) {
        throw ConfigError("config file parse error");
    }

    if (!config.contains("config") || !config["config"].is_object()) {
        throw ConfigError("config file is empty");
    }

    if (!config["config"].contains("version") || !config["config"]["version"].is_string()) {
        throw ConfigError("config.json has incorrect file version");
    }

    if (config["config"]["version"].get<std::string>() != kExpectedVersion) {
        throw ConfigError("config.json has incorrect file version");
    }

    return config;
}

std::vector<std::string> ConverterJSON::GetTextDocuments() {
    std::vector<std::string> documents;
    json config = LoadConfig();
    const std::filesystem::path config_dir = std::filesystem::path(ResolveConfigPath()).parent_path();

    // Вывод названия поискового движка
    if (config["config"].contains("name") && config["config"]["name"].is_string()) {
        std::cout << "Starting " << config["config"]["name"] << std::endl;
    }

    // Чтение списка файлов
    if (config.contains("files") && config["files"].is_array()) {
        for (const auto& file_path : config["files"]) {
            if (!file_path.is_string()) {
                documents.emplace_back();
                continue;
            }
            const std::filesystem::path configured_path = file_path.get<std::string>();
            const std::filesystem::path resolved_path = configured_path.is_absolute()
                    ? configured_path
                    : (config_dir / configured_path).lexically_normal();
            std::ifstream file(resolved_path);

            if (!file.is_open()) {
                std::cerr << "Warning: cannot open file " << resolved_path.string() << std::endl;
                // Keep doc index aligned with config.files even if file is missing.
                documents.emplace_back();
                continue;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            documents.push_back(buffer.str());
            file.close();
        }
    }

    return documents;
}

int ConverterJSON::GetResponsesLimit() {
    json config = LoadConfig();

    if (config["config"].contains("max_responses") && config["config"]["max_responses"].is_number_integer()) {
        return config["config"]["max_responses"];
    }

    return 5; // значение по умолчанию
}

std::vector<std::string> ConverterJSON::GetRequests() {
    std::vector<std::string> requests;

    const std::filesystem::path config_dir = std::filesystem::path(ResolveConfigPath()).parent_path();
    const std::filesystem::path requests_path = (config_dir / "requests.json").lexically_normal();

    std::ifstream requests_file(requests_path);
    if (!requests_file.is_open()) {
        std::cerr << "Warning: requests.json file is missing: " << requests_path.string() << std::endl;
        return requests;
    }

    json requests_json;
    try {
        requests_file >> requests_json;
    } catch (const json::parse_error&) {
        std::cerr << "Error parsing requests.json" << std::endl;
        return requests;
    }

    if (requests_json.find("requests") != requests_json.end()) {
        for (const auto& request : requests_json["requests"]) {
            requests.push_back(request);
        }
    }

    return requests;
}

void ConverterJSON::putAnswers(std::vector<std::vector<std::pair<int, float>>> answers) {
    json result;
    json answers_json;

    for (size_t i = 0; i < answers.size(); ++i) {
        std::string request_id = "request" + std::string(3 - std::to_string(i + 1).length(), '0') + std::to_string(i + 1);

        if (answers[i].empty()) {
            answers_json[request_id]["result"] = false;
        } else if (answers[i].size() == 1) {
            answers_json[request_id]["result"] = true;
            answers_json[request_id]["docid"] = answers[i][0].first;
            answers_json[request_id]["rank"] = answers[i][0].second;
        } else {
            answers_json[request_id]["result"] = true;

            json relevance_array = json::array();
            for (const auto& answer : answers[i]) {
                json relevance_item;
                relevance_item["docid"] = answer.first;
                relevance_item["rank"] = answer.second;
                relevance_array.push_back(relevance_item);
            }
            answers_json[request_id]["relevance"] = relevance_array;
        }
    }

    result["answers"] = answers_json;

    const std::filesystem::path config_dir = std::filesystem::path(ResolveConfigPath()).parent_path();
    const std::filesystem::path answers_path = (config_dir / "answers.json").lexically_normal();
    std::ofstream output_file(answers_path);
    output_file << result.dump(4);
    output_file.close();
}

