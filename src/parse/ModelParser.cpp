#include "../data/Model.h"
#include "../parse/json/json.hpp"
#include <fstream>
#include <iostream>

namespace {
    Vec3 parseVec3(const nlohmann::json& json) {
        return Vec3(
            json.value("x", 0.0f),
            json.value("y", 0.0f),
            json.value("z", 0.0f)
        );
    }

    Vec4 parseVec4(const nlohmann::json& json) {
        return Vec4(
            json.value("x", 0.0f),
            json.value("y", 0.0f),
            json.value("z", 0.0f),
            json.value("w", 1.0f)
        );
    }

    Vec2 parseVec2(const nlohmann::json& json) {
        return Vec2(
            json.value("x", 0.0f),
            json.value("y", 0.0f)
        );
    }

    FaceLayout parseFaceLayout(const nlohmann::json& json) {
        FaceLayout layout;
        if (json.contains("offset")) {
            layout.offset = parseVec2(json["offset"]);
        }
        if (json.contains("mirror")) {
            layout.mirror = parseVec2(json["mirror"]);
        }
        layout.angle = json.value("angle", 0);
        return layout;
    }

    NodeShape parseNodeShape(const nlohmann::json& json) {
        NodeShape shape;

        shape.visible = json.value("visible", true);
        shape.doubleSided = json.value("doubleSided", false);
        shape.shadingMode = json.value("shadingMode", "standard");
        shape.type = json.value("type", "none");

        if (json.contains("offset")) {
            shape.offset = parseVec3(json["offset"]);
        }
        if (json.contains("stretch")) {
            shape.stretch = parseVec3(json["stretch"]);
        }

        if (json.contains("settings")) {
            const auto& settings = json["settings"];
            if (settings.contains("size")) {
                shape.settings.size = parseVec3(settings["size"]);
            }
            if (settings.contains("normal")) {
                shape.settings.normal = settings["normal"];
            }
            shape.settings.isPiece = settings.value("isPiece", false);
        }

        if (json.contains("textureLayout")) {
            const auto& texLayout = json["textureLayout"];
            for (auto it = texLayout.begin(); it != texLayout.end(); ++it) {
                shape.textureLayout[it.key()] = parseFaceLayout(it.value());
            }
        }

        return shape;
    }

    void parseNodeRecursive(const nlohmann::json& json, ModelNodeJson& node) {
        if (json.contains("name")) {
            node.name = json["name"];
        }

        if (json.contains("position")) {
            node.position = parseVec3(json["position"]);
        }

        if (json.contains("orientation")) {
            node.orientation = parseVec4(json["orientation"]);
        }

        if (json.contains("shape")) {
            node.shape = parseNodeShape(json["shape"]);
        }

        if (json.contains("children") && json["children"].is_array()) {
            for (const auto& childJson : json["children"]) {
                ModelNodeJson child;
                parseNodeRecursive(childJson, child);
                node.children.push_back(child);
            }
        }
    }
}

ModelJson parseBlockyModel(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open blockymodel file: " << filepath << "\n";
        return ModelJson();
    }

    nlohmann::json jsonData;
    try {
        jsonData = nlohmann::json::parse(file);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse blockymodel JSON: " << e.what() << "\n";
        return ModelJson();
    }

    ModelJson modelJson;

    if (jsonData.contains("lod")) {
        modelJson.lod = jsonData["lod"];
    }

    if (jsonData.contains("nodes") && jsonData["nodes"].is_array()) {
        for (const auto& nodeJson : jsonData["nodes"]) {
            ModelNodeJson node;
            parseNodeRecursive(nodeJson, node);
            modelJson.nodes.push_back(node);
        }
    }

    return modelJson;
}