#include "../data/Model.h"
#include "../parse/ModelParser.h"
#include "../parse/json/json.hpp"
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>

Model* ModelRegistry::loadModel(const std::string& modelName) {
    if (hasModel(modelName)) {
        return getModel(modelName);
    }

    std::string modelPath = findModelPath(modelName);
    if (modelPath.empty()) {
        std::cerr << "Could not find model path for: " << modelName << "\n";
        return nullptr;
    }

    if (modelPath == "CUBE") {
        // TODO: Handle cube models
        return nullptr;
    }

    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "Model file does not exist: " << modelPath << "\n";
        return nullptr;
    }

    ModelJson modelJson = parseBlockyModel(modelPath);

    Model* model = new Model();
    ModelInitializer::parse(modelJson, &nodeNameManager, *model);

    models[modelName] = model;
    return model;
}

std::string ModelRegistry::findModelPath(const std::string& modelName) {
    std::string itemsPath = assetPath + "/Server/Item/Items";
    std::string targetFile = modelName + ".json";

    std::function<std::string(const std::filesystem::path&)> searchRecursive =
        [&](const std::filesystem::path& dir) -> std::string {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().filename() == targetFile) {
                return entry.path().string();
            }
            else if (entry.is_directory()) {
                std::string result = searchRecursive(entry.path());
                if (!result.empty()) {
                    return result;
                }
            }
        }
        return "";
        };

    std::filesystem::path foundFilePath = searchRecursive(itemsPath);
    if (foundFilePath == "") return "";

    std::ifstream file(foundFilePath);
    nlohmann::json jsonData = nlohmann::json::parse(file);

    if (!jsonData.contains("BlockType") || !jsonData["BlockType"].contains("DrawType")) {
        return "";
    }

    std::string drawType = jsonData["BlockType"]["DrawType"];

    if (drawType == "Cube") {
        return "CUBE";
    }
    else if (drawType == "Model") {
        if (jsonData["BlockType"].contains("CustomModel")) {
            std::string customModelPath = jsonData["BlockType"]["CustomModel"];
            return assetPath + "/Common/" + customModelPath;
        }
    }

    return "";
}

std::string ModelRegistry::findTexturePath(const std::string& modelName) {
    std::string itemsPath = assetPath + "/Server/Item/Items";
    std::string targetFile = modelName + ".json";

    std::function<std::string(const std::filesystem::path&)> searchRecursive =
        [&](const std::filesystem::path& dir) -> std::string {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().filename() == targetFile) {
                return entry.path().string();
            }
            else if (entry.is_directory()) {
                std::string result = searchRecursive(entry.path());
                if (!result.empty()) {
                    return result;
                }
            }
        }
        return "";
        };

    std::filesystem::path foundFilePath = searchRecursive(itemsPath);
    if (foundFilePath == "") return "";

    std::ifstream file(foundFilePath);
    nlohmann::json jsonData = nlohmann::json::parse(file);

    if (!jsonData.contains("BlockType")) return "";

    std::string texturePath;

    if (jsonData["BlockType"].contains("CustomModelTexture") &&
        jsonData["BlockType"]["CustomModelTexture"].is_array() &&
        !jsonData["BlockType"]["CustomModelTexture"].empty()) {
        texturePath = jsonData["BlockType"]["CustomModelTexture"][0]["Texture"];
    }
    else if (jsonData["BlockType"].contains("Textures") &&
        jsonData["BlockType"]["Textures"].is_array() &&
        !jsonData["BlockType"]["Textures"].empty()) {
        texturePath = jsonData["BlockType"]["Textures"][0]["All"];
    }
    else {
        return "";
    }

    return assetPath + "/Common/" + texturePath;
}

Model* ModelRegistry::getModel(const std::string& modelName) {
    auto it = models.find(modelName);
    if (it != models.end()) {
        return it->second;
    }
    return nullptr;
}

bool ModelRegistry::hasModel(const std::string& modelName) const {
    return models.find(modelName) != models.end();
}