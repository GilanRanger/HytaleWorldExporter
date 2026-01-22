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

    if (modelPath == "EMPTY") {
        Model* model = new Model();
        models[modelName] = model;
        return model;
    }

    Model* model = new Model();

    if (modelPath == "CUBE") {
        ModelNode node;
        node.nameId = nodeNameManager.getOrAddNameId("Cube");
        node.position = Vec3(0, 0, 0);
        node.orientation = Vec4::Identity();
        node.offset = Vec3(0, 0, 0);
        node.stretch = Vec3(1, 1, 1);
        node.type = ModelNode::ShapeType::Box;
        node.size = Vec3(32, 32, 32);
        node.visible = true;
        node.doubleSided = false;
        node.shadingMode = ShadingMode::Standard;
        node.gradientId = 0;
        node.textureLayout.resize(6);
        model->addNode(node);
    }
    else {
        if (!std::filesystem::exists(modelPath)) {
            std::cerr << "Model file does not exist: " << modelPath << "\n";
            return nullptr;
        }

        ModelJson modelJson = parseBlockyModel(modelPath);
        ModelInitializer::parse(modelJson, &nodeNameManager, *model);
    }

    std::string texturePath = findTexturePath(modelName);
    if (!texturePath.empty() && texturePath != "EMPTY") {
        const AtlasRegion* region = textureRegistry->getTextureRegion(texturePath);

        if (region) {
            float sourceTexWidth = static_cast<float>(region->pixelWidth);
            float sourceTexHeight = static_cast<float>(region->pixelHeight);

            float atlasWidthInUV = region->uvMax.u - region->uvMin.u;
            float atlasHeightInUV = region->uvMax.v - region->uvMin.v;

            for (int i = 0; i < model->nodeCount; i++) {
                ModelNode& node = model->allNodes[i];

                for (auto& layout : node.textureLayout) {
                    // Convert pixel offsets to normalized UVs in source texture space
                    float normalizedU = layout.offset.u / sourceTexWidth;
                    float normalizedV = layout.offset.v / sourceTexHeight;

                    // Transform to atlas space
                    layout.offset.u = region->uvMin.u + (normalizedU * atlasWidthInUV);
                    layout.offset.v = region->uvMin.v + (normalizedV * atlasHeightInUV);
                }

                node.uvMin = region->uvMin;
                node.uvMax = region->uvMax;
            }
        }
    }

    models[modelName] = model;
    return model;
}

std::string ModelRegistry::findModelPath(const std::string& modelName) {
    std::string itemsPath = assetPath + "/Server/Item/Items";
    std::string targetFile = modelName + ".json";

    if (modelName == "Empty") return "EMPTY";

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

    // Check for CustomModel
    if (jsonData.contains("BlockType") && jsonData["BlockType"].contains("CustomModel")) {
        std::string customModelPath = jsonData["BlockType"]["CustomModel"];
        return assetPath + "/Common/" + customModelPath;
    }

    // Check for DrawType
    if (jsonData.contains("BlockType") && jsonData["BlockType"].contains("DrawType")) {
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
    }

    // Handle parent inheritance
    if (jsonData.contains("Parent")) {
        std::string parentName = jsonData["Parent"];
        return findModelPath(parentName);
    }

    return "";
}

std::string ModelRegistry::findTexturePath(const std::string& modelName) {
    std::string itemsPath = assetPath + "/Server/Item/Items";
    std::string targetFile = modelName + ".json";

    if (modelName == "Empty") return "EMPTY";

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